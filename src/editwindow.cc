#define Uses_TScrollBar
#define Uses_MsgBox
#include <tvision/tv.h>

#include "editwindow.h"
#include "fileedit.h"

EditorWindow::EditorWindow(const TRect &bounds, std::string_view aFileName) :
    TWindow(bounds, nullptr, wnNoNumber),
    TWindowInit(&initFrame),
    drawing(false),
    editorView(editorBounds())
{
    options |= ofTileable;
    setState(sfShadow, False);

    editorView.hide();

    vScrollBar = new TScrollBar(TRect( size.x - 1, 1, size.x, size.y - 1 ));
    vScrollBar->hide();
    insert(vScrollBar);

    leftMargin = new TDrawSubView(TRect( 1, 1, 6, size.y - 1 ), editorView);
    leftMargin->options |= ofFramed;
    leftMargin->growMode = gfGrowHiY | gfFixed;
    insert(leftMargin);

    docView = new FileEditor( aFileName,
                              TRect( 7, 1, size.x - 1, size.y - 1 ),
                              editorView,
                              editor,
                              *this );
    insert(docView);

    setUpEditor();
}

TRect EditorWindow::editorBounds() const
{
    // Editor size: the window's inside minus the line numbers frame.
    TRect r = getExtent().grow(-1, -1);
    r.b.x--;
    return r;
}

void EditorWindow::setUpEditor()
{
    // Editor should take into account the size of docView.
    editor.setWindow(&editorView);
    // But should send notifications to this window.
    editor.setParent(this);

    // Colors
    uchar color = 0x1E; // Blue & Light Yellow.
    uchar colorSel = 0x71; // White & Blue.
    editorView.setFillColor(color); // Screw palettes, they are too hard to understand.
    editor.setStyleColor(STYLE_DEFAULT, color);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    editor.setSelectionColor(colorSel);

    // Line numbers
    int marginWidth = 5;
    editor.setStyleColor(STYLE_LINENUMBER, color);
    editor.WndProc(SCI_SETMARGINS, 1, 0U);
    editor.WndProc(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    editor.WndProc(SCI_SETMARGINWIDTHN, 0, marginWidth);

    // The document view does not contain the margin.
    docView->setDelta({marginWidth, 0});

    redrawEditor();
}

void EditorWindow::redrawEditor()
{
    if (!drawing) {
        drawing = true;
        lock();
        editor.changeSize();
        editor.draw(editorView);
        leftMargin->drawView();
        docView->drawView();
        vScrollBar->drawView();
        unlock();
        drawing = false;
    }
}

void EditorWindow::setActive(Boolean enable)
{
    vScrollBar->setState(sfVisible, enable);
}

void EditorWindow::handleEvent(TEvent &ev) {
    if (ev.what == evBroadcast && ev.message.command == cmScrollBarChanged) {
        if (scrollBarChanged((TScrollBar *) ev.message.infoPtr)) {
            redrawEditor();
            clearEvent(ev);
        }
    }
    TWindow::handleEvent(ev);
}

void EditorWindow::changeBounds(const TRect &bounds)
{
    lock();
    lockSubViews();
    TWindow::changeBounds(bounds);
    editorView.changeBounds(editorBounds());
    unlockSubViews();
    redrawEditor();
    unlock();
}

Boolean EditorWindow::valid(ushort command)
{
    if (command == cmValid && !error.empty()) {
        messageBox(error.c_str(), mfError | mfOKButton);
        return False;
    }
    return True;
}

const char* EditorWindow::getTitle(short)
{
    if (!title.empty())
        return title.c_str();
    if (!*docView->fileName)
        return nullptr;
    return docView->fileName;
}

void EditorWindow::lockSubViews()
{
    for (auto *v : std::initializer_list<TView *> {docView, leftMargin, vScrollBar})
        v->setState(sfExposed, False);
}

void EditorWindow::unlockSubViews()
{
    for (auto *v : std::initializer_list<TView *> {docView, leftMargin, vScrollBar})
        v->setState(sfExposed, True);
}

void EditorWindow::scrollBarEvent(TEvent ev)
{
    vScrollBar->handleEvent(ev);
}

bool EditorWindow::scrollBarChanged(TScrollBar *bar)
{
    if (bar == vScrollBar) {
        editor.WndProc(SCI_SETFIRSTVISIBLELINE, bar->value, 0U);
        return true;
    }
    return false;
}

void EditorWindow::setVerticalScrollPos(int delta, int limit, int size)
{
    vScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
}
