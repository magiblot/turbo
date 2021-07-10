#define Uses_TGroup
#define Uses_TScrollBar
#include <tvision/tv.h>

#include <turbo/turbo.h>
#include "util.h"

namespace turbo {

EditorState::EditorState() :
    EditorState(*new Editor)
{
}

EditorState::EditorState(Editor &aEditor) :
    editor(aEditor)
{
    // Editor should send notifications to this object.
    editor.setParent(this);
    // Set color defaults.
    theming.resetStyles(editor);

    // Dynamic horizontal scroll.
    editor.WndProc(SCI_SETSCROLLWIDTHTRACKING, true, 0U);
    editor.WndProc(SCI_SETSCROLLWIDTH, 1, 0U);
    editor.WndProc(SCI_SETXCARETPOLICY, CARET_EVEN, 0);
    // Trick so that the scroll width gets computed.
    editor.WndProc(SCI_SETFIRSTVISIBLELINE, 1, 0U);
    editor.WndProc(SCI_SETFIRSTVISIBLELINE, 0, 0U);

    // Enable wrapping markers
    editor.WndProc(SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_END, 0U);

    // Indentation
    editor.WndProc(SCI_SETUSETABS, false, 0U);
    editor.WndProc(SCI_SETINDENT, 4, 0U);
    editor.WndProc(SCI_SETTABINDENTS, true, 0U);
    editor.WndProc(SCI_SETBACKSPACEUNINDENTS, true, 0U);

    // Line numbers
    editor.WndProc(SCI_SETMARGINS, 1, 0U);
    editor.WndProc(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    updateMarginWidth();

    // Savepoint and undo buffer.
    editor.WndProc(SCI_EMPTYUNDOBUFFER, 0U, 0U);
    editor.WndProc(SCI_SETSAVEPOINT, 0U, 0U);
}

EditorState::~EditorState()
{
    delete &editor;
}

void EditorState::associate( EditorView *aView, LeftMarginView *aLeftMargin,
                             TScrollBar *aHScrollBar, TScrollBar *aVScrollBar )
{
    disassociate();
    if (aView)
    {
        if (aView->state)
            aView->state->disassociate();
        aView->state = this;
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

void EditorState::disassociate()
// Pre: if view != nullptr, view->state == this.
// Post: if view != nullptr, it is sized as if the line numbers were hidden.
{
    if (view)
    {
        if (leftMargin)
        {
            TRect r = view->getBounds();
            r.a.x = leftMargin->getBounds().a.x;
            view->setBounds(r);
        }
        view->state = nullptr;
    }
    view = nullptr;
    leftMargin = nullptr;
    hScrollBar = nullptr;
    vScrollBar = nullptr;
}

TPoint EditorState::getEditorSize()
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

void EditorState::scrollTo(TPoint delta)
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


void EditorState::redraw()
{
    auto size = getEditorSize();
    if (redraw({0, 0, size.x, size.y}))
        invalidatedArea.clear();
}

void EditorState::partialRedraw()
{
    if (redraw(invalidatedArea))
        invalidatedArea.clear();
}

void EditorState::invalidate(TRect area)
{
    if (invalidatedArea.empty())
        invalidatedArea = area;
    else
        invalidatedArea.Union(area);
}

bool EditorState::redraw(const TRect &area)
{
    if ( !drawLock && 0 <= area.a.x && area.a.x < area.b.x
                   && 0 <= area.a.y && area.a.y < area.b.y )
    {
        drawLock = true;
        updateMarginWidth();
        if (!resizeLock)
        {
            editor.changeSize();
            theming.updateBraces(editor); // May mutate 'invalidatedArea', which may be 'area'.
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
        editor.paint(surface, paintArea);
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

void EditorState::drawViews()
{
    forEach<TView>({vScrollBar, hScrollBar, leftMargin, view}, [&] (auto &p) {
        if (p.owner) p.owner->lock();
    });
    forEach<TView>({vScrollBar, hScrollBar, leftMargin, view}, [&] (auto &p) {
        p.drawView();
        if (p.owner) p.owner->unlock();
    });
}

void EditorState::updateMarginWidth()
{
    int width = lineNumbers.update(editor);
    if (leftMargin)
    {
        TRect mr = leftMargin->getBounds();
        mr.b.x = mr.a.x + width;
        leftMargin->setBounds(mr);
        if (view)
        {
            TRect vr = view->getBounds();
            vr.a.x = mr.b.x + leftMargin->distanceFromView*(width != 0);
            view->setBounds(vr);
            view->delta = {width, 0};
        }
    }
}

bool EditorState::handleScrollBarChanged(TScrollBar *s)
{
    if (s == hScrollBar)
    {
        editor.WndProc(SCI_SETXOFFSET, s->value, 0U);
        return true;
    }
    else if (s == vScrollBar)
    {
        editor.WndProc(SCI_SETFIRSTVISIBLELINE, s->value, 0U);
        return true;
    }
    return false;
}

void EditorState::setHorizontalScrollPos(int delta, int limit)
{
    if (view && hScrollBar)
    {
        auto size = view->size.x;
        hScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
    }
}

void EditorState::setVerticalScrollPos(int delta, int limit)
{
    if (view && vScrollBar)
    {
        auto size = view->size.y;
        vScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
    }
}

bool EditorState::inSavePoint()
{
    return editor.WndProc(SCI_GETMODIFY, 0U, 0U) == 0;
}

} // namespace turbo
