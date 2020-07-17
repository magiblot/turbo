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

    leftSideBar = new TView( {{1, 1}, {6, size.y - 1}} );
    leftSideBar->options |= ofFramed;
    leftSideBar->growMode = gfGrowHiY | gfFixed;
    insert(leftSideBar);

    bottomArea = new TView( {{1, size.y - 1}, {size.x - 1, size.y - 1}} );
    bottomArea->options |= ofFramed;
    bottomArea->growMode = gfGrowAll & ~gfGrowLoX;
    bottomArea->hide();
    insert(bottomArea);

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
    editor.setStyleColor(STYLE_DEFAULT, color);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U);
    editor.setSelectionColor(colorSel);

    redrawEditor();
}

void EditorWindow::redrawEditor()
{
    lock();
    // Draw on docView's surface
    docView->doUpdate();
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
