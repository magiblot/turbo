#define Uses_TScrollBar
#include <tvision/tv.h>

#include "editwindow.h"

EditorWindow::EditorWindow(const TRect &bounds) :
    TWindow(bounds, "Editor Window", wnNoNumber),
    TWindowInit(&initFrame),
    needsRedraw(false)
{
    options |= ofTileable;
    setState(sfShadow, False);

    TScrollBar *hScrollBar =
        new TScrollBar( TRect( 18, size.y - 1, size.x - 2, size.y ) );
    hScrollBar->hide();
    insert(hScrollBar);

    TScrollBar *vScrollBar =
        new TScrollBar( TRect( size.x - 1, 1, size.x, size.y - 1 ) );
    vScrollBar->hide();
    insert(vScrollBar);

    leftMargin = new TDrawableView( {{1, 1}, {6, size.y - 1}} );
    leftMargin->options |= ofFramed;
    leftMargin->growMode = gfGrowHiY | gfFixed;
    insert(leftMargin);

    docView = new DocumentView( {{7, 1}, {size.x - 1, size.y - 1}}, editor );
    insert(docView);

    setUpEditor();

}

void EditorWindow::setUpEditor()
{
    // Editor should take into account the size of docView.
    editor.setWindow(docView);
    // But send notifications to this window.
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
    lock();
    // Draw on docView's surface
    docView->doUpdate();
    // Temporally enable margin width to draw line numbers
    editor.WndProc(SCI_SETMARGINWIDTHN, 0, 5);
    editor.draw(*leftMargin);
    editor.WndProc(SCI_SETMARGINWIDTHN, 0, 0);
    leftMargin->drawView();
    unlock();
}

void EditorWindow::handleEvent(TEvent &ev)
{
    TWindow::handleEvent(ev);
    if (needsRedraw) {
        redrawEditor();
        needsRedraw = false;
    }
}

void EditorWindow::changeBounds(const TRect &bounds)
{
    TWindow::changeBounds(bounds);
    // If redrawEditor() did no changes to margin sizes, the following
    // would be necessary:
//     editor.changeSize();
    redrawEditor();
}

void EditorWindow::notify(SCNotification scn)
{
    switch (scn.nmhdr.code) {
        default:
            needsRedraw = true;
            break;
    }
}
