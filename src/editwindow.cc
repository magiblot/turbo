#define Uses_TScrollBar
#include <tvision/tv.h>

#include "editwindow.h"

EditorWindow::EditorWindow(const TRect &bounds) :
    TWindow(bounds, "Editor Window", wnNoNumber),
    TWindowInit(&initFrame),
    drawing(false)
{
    options |= ofTileable;
    setState(sfShadow, False);

    vScrollBar = new TScrollBar(TRect( size.x - 1, 1, size.x, size.y - 1 ));
    vScrollBar->hide();
    insert(vScrollBar);

    leftMargin = new TDrawableView(TRect( 1, 1, 6, size.y - 1 ));
    leftMargin->options |= ofFramed;
    leftMargin->growMode = gfGrowHiY | gfFixed;
    insert(leftMargin);

    docView = new DocumentView( TRect( 7, 1, size.x - 1, size.y - 1 ),
                                editor,
                                *this );
    insert(docView);

    setUpEditor();
}

void EditorWindow::setUpEditor()
{
    // Editor should take into account the size of docView.
    editor.setWindow(docView);
    // But should send notifications to this window.
    editor.setParent(this);

    // Colors
    uchar color = 0x1E; // Blue & Light Yellow.
    uchar colorSel = 0x71; // White & Blue.
    docView->setFillColor(color); // Screw palettes, they are too hard to understand.
    leftMargin->setFillColor(color);
    editor.setStyleColor(STYLE_DEFAULT, color);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    editor.setSelectionColor(colorSel);

    // Line numbers
    editor.setStyleColor(STYLE_LINENUMBER, color);
    editor.WndProc(SCI_SETMARGINS, 1, 0U);
    editor.WndProc(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    editor.WndProc(SCI_SETMARGINWIDTHN, 0, 0);

    redrawEditor();
}

void EditorWindow::redrawEditor()
{
    if (!drawing) {
        drawing = true;
        lock();
        // Temporally enable margin width to draw line numbers. This must be
        // done first or else mouse positions will be later clipped
        // as if the margin was still being shown.
        editor.WndProc(SCI_SETMARGINWIDTHN, 0, 5);
        editor.draw(*leftMargin);
        editor.WndProc(SCI_SETMARGINWIDTHN, 0, 0);
        leftMargin->drawView();
        // Draw on docView's surface
        docView->doUpdate();
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
