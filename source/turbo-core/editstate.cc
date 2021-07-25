#define Uses_TGroup
#define Uses_TScrollBar
#include <tvision/tv.h>

#include <turbo/turbo.h>
#include <turbo/util.h>
#include "utils.h"

namespace turbo {

EditorState::EditorState(Scintilla &aScintilla) noexcept :
    scintilla(aScintilla)
{
    // Editor should send notifications to this object.
    setParent(scintilla, this);
    // Set color defaults.
    theming.resetStyles(scintilla);

    // Dynamic horizontal scroll.
    call(scintilla, SCI_SETSCROLLWIDTHTRACKING, true, 0U);
    call(scintilla, SCI_SETSCROLLWIDTH, 1, 0U);
    call(scintilla, SCI_SETXCARETPOLICY, CARET_EVEN, 0);
    // Trick so that the scroll width gets computed.
    call(scintilla, SCI_SETFIRSTVISIBLELINE, 1, 0U);
    call(scintilla, SCI_SETFIRSTVISIBLELINE, 0, 0U);

    // Enable wrapping markers
    call(scintilla, SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_END, 0U);

    // Indentation
    call(scintilla, SCI_SETUSETABS, false, 0U);
    call(scintilla, SCI_SETINDENT, 4, 0U);
    call(scintilla, SCI_SETTABINDENTS, true, 0U);
    call(scintilla, SCI_SETBACKSPACEUNINDENTS, true, 0U);

    // Line numbers
    call(scintilla, SCI_SETMARGINS, 1, 0U);
    call(scintilla, SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    updateMarginWidth();

    // Savepoint and undo buffer.
    call(scintilla, SCI_EMPTYUNDOBUFFER, 0U, 0U);
    call(scintilla, SCI_SETSAVEPOINT, 0U, 0U);
}

EditorState::~EditorState()
{
    destroyScintilla(scintilla);
}

void EditorState::associate( EditorParent *aParent,
                             EditorView *aView, LeftMarginView *aLeftMargin,
                             TScrollBar *aHScrollBar, TScrollBar *aVScrollBar ) noexcept
{
    disassociate();
    parent = aParent;
    if (aView)
    {
        if (aView->editorState)
            aView->editorState->disassociate();
        aView->editorState = this;
        aView->state |= sfCursorVis;
    }
    view = aView;
    if (aView && aLeftMargin)
    {
        // Place the margin to the left of the view, as if it was hidden.
        // If necessary, it will be made visible during redraw().
        TRect r = aView->getBounds();
        r.b.x = r.a.x;
        aLeftMargin->setBounds(r);
    }
    leftMargin = aLeftMargin;
    hScrollBar = aHScrollBar;
    vScrollBar = aVScrollBar;
}

void EditorState::disassociate() noexcept
// Pre: if view != nullptr, view->editorState == this.
// Post: if view != nullptr && leftMargin != nullptr, they are sized as if
//       the line numbers were hidden.
{
    parent = nullptr;
    if (view)
    {
        if (leftMargin)
        {
            TRect r = view->getBounds();
            r.a.x = leftMargin->getBounds().a.x;
            view->setBounds(r);
            leftMargin->size.x = 0;
        }
        view->editorState = nullptr;
        view->state &= ~sfCursorVis;
    }
    view = nullptr;
    leftMargin = nullptr;
    hScrollBar = nullptr;
    vScrollBar = nullptr;
}

TPoint EditorState::getEditorSize() noexcept
{
    if (view)
        return {
            view->size.x + (leftMargin ? leftMargin->size.x : 0),
            view->size.y,
        };
    return {0, 0};
}

void EditorState::scrollBarEvent(TEvent &ev)
{
    // TScrollBar::handleEvent leads to a cmScrollBarChanged being messaged,
    // which EditorView handles with a call to redraw(). Hold the draw lock
    // to prevent such redraw from happening.
    bool lastDrawLock = drawLock;
    drawLock = true;
    if (hScrollBar)
        hScrollBar->handleEvent(ev);
    if (vScrollBar)
        vScrollBar->handleEvent(ev);
    drawLock = lastDrawLock;
}

void EditorState::scrollTo(TPoint delta) noexcept
{
    // TScrollBar::setValue leads to a cmScrollBarChanged being messaged,
    // which EditorView handles with a call to redraw(). Hold the draw lock
    // to prevent such redraw from happening.
    bool lastDrawLock = drawLock;
    drawLock = true;
    if (hScrollBar)
        hScrollBar->setValue(delta.x);
    if (vScrollBar)
        vScrollBar->setValue(delta.y);
    drawLock = lastDrawLock;
}


void EditorState::redraw() noexcept
{
    auto size = getEditorSize();
    if (redraw({0, 0, size.x, size.y}))
        invalidatedArea.clear();
}

void EditorState::partialRedraw() noexcept
{
    if (redraw(invalidatedArea))
        invalidatedArea.clear();
}

void EditorState::invalidate(TRect area) noexcept
{
    if (invalidatedArea.empty())
        invalidatedArea = area;
    else
        invalidatedArea.Union(area);
}

bool EditorState::redraw(const TRect &area) noexcept
{
    if ( !drawLock && 0 <= area.a.x && area.a.x < area.b.x
                   && 0 <= area.a.y && area.a.y < area.b.y )
    {
        drawLock = true;
        updateMarginWidth();
        if (!resizeLock)
        {
            changeSize(scintilla);
            theming.updateBraces(scintilla); // May mutate 'invalidatedArea', which may be 'area'.
        }
        auto size = getEditorSize();
        TRect paintArea;
        if (surface.size != size)
        {
            surface.resize(size);
            // We need to redraw the whole editor because it has been resized.
            paintArea = {{0, 0}, size};
        }
        else
            paintArea = area;
        paint(scintilla, surface, paintArea);
        forEach<TSurfaceView>({leftMargin, view}, [&] (auto &p) {
            p.surface = &surface;
        });
        drawViews();
        forEach<TSurfaceView>({leftMargin, view}, [&] (auto &p) {
            p.surface = nullptr;
        });
        drawLock = false;
        return true;
    }
    return false;
}

void EditorState::drawViews() noexcept
{
    forEach<TView>({vScrollBar, hScrollBar, leftMargin, view}, [&] (auto &p) {
        p.drawView();
    });
    if (parent)
        parent->handleNotification(ncPainted, *this);
}

void EditorState::updateMarginWidth() noexcept
{
    int width = lineNumbers.update(scintilla);
    if (leftMargin)
    {
        TRect mr = leftMargin->getBounds();
        mr.b.x = mr.a.x + width;
        leftMargin->setBounds(mr);
        if (view)
        {
            TRect vr = view->getBounds();
            vr.a.x = mr.b.x + leftMargin->distanceFromView * (width != 0);
            view->setBounds(vr);
            view->delta = {width, 0};
        }
    }
}

bool EditorState::handleScrollBarChanged(TScrollBar *s)
{
    if (s == hScrollBar)
    {
        call(scintilla, SCI_SETXOFFSET, s->value, 0U);
        return true;
    }
    else if (s == vScrollBar)
    {
        call(scintilla, SCI_SETFIRSTVISIBLELINE, s->value, 0U);
        return true;
    }
    return false;
}

void EditorState::handleNotification(const SCNotification &scn)
{
    switch (scn.nmhdr.code)
    {
        case SCN_CHARADDED:
            if (scn.ch == '\n')
                autoIndent.applyToCurrentLine(scintilla);
            break;
    }
}

void EditorState::setHorizontalScrollPos(int delta, int limit) noexcept
{
    if (view && hScrollBar)
    {
        auto size = view->size.x;
        hScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
    }
}

void EditorState::setVerticalScrollPos(int delta, int limit) noexcept
{
    if (view && vScrollBar)
    {
        auto size = view->size.y;
        vScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
    }
}

bool EditorState::inSavePoint()
{
    return call(scintilla, SCI_GETMODIFY, 0U, 0U) == 0;
}

} // namespace turbo
