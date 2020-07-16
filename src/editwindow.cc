#define Uses_TScrollBar
#include <tvision/tv.h>

#include "editwindow.h"

EditorWindow::EditorWindow(const TRect &bounds) :
    TWindow(bounds, "Editor Window", wnNoNumber),
    TWindowInit(&initFrame)
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

}
