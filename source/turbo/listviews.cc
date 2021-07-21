#define Uses_TScrollBar
#include <tvision/tv.h>

#include "listviews.h"

#define cpListWindow "\x32\x32\x34\x37\x36\x32\x33"

ListWindow::ListWindow(const TRect &bounds, const char *aTitle, List &aList) :
    ListWindow(bounds, aTitle, aList, &initViewer<ListView>)
{
}

ListWindow::ListWindow( const TRect &bounds, const char *aTitle, List &aList,
                        std::function<ListView *(TRect, TWindow *, List &)> &&cListViewer ) :
    TWindowInit(&initFrame),
    TWindow(bounds, aTitle, wnNoNumber),
    list(aList)
{
    flags = wfClose | wfMove;
    viewer = cListViewer(getExtent(), this, aList);
    insert(viewer);
}

TPalette& ListWindow::getPalette() const
{
    static TPalette palette(cpListWindow, sizeof(cpListWindow) - 1);
    return palette;
}

void* ListWindow::getSelected()
{
    return viewer->getSelected();
}

void ListWindow::handleEvent(TEvent& event)
{
    TWindow::handleEvent(event);
    if (event.what == evMouseDown && !mouseInView(event.mouse.where)) {
        endModal(cmCancel);
        clearEvent(event);
    }
}

#define cpListViewer "\x06\x06\x07\x06\x06"

ListView::ListView( const TRect& bounds,
                    TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar,
                    ListWindow::List &aList ) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    list(aList)
{
    growMode = gfGrowHiX | gfGrowHiY;
    setRange(list.size());
    if (range > 1)
        focusItem(1);
    hScrollBar->setRange(0, list.measureWidth() - size.x + 3);
}

TPalette& ListView::getPalette() const
{
    static TPalette palette(cpListViewer, sizeof(cpListViewer) - 1);
    return palette;
}

void ListView::getText(char *dest, short item, short maxChars)
{
    std::string_view text = list.getText(list.at(item));
    size_t count = std::min<size_t>(maxChars, text.size());
    strnzcpy(dest, text.data(), count+1);
}

void* ListView::getSelected()
{
    return list.at(focused);
}

void ListView::handleEvent(TEvent& event)
{
    if ( (event.what == evMouseDown && (event.mouse.eventFlags & meDoubleClick)) ||
         (event.what == evKeyDown && event.keyDown.keyCode == kbEnter) ) {
        endModal(cmOK);
        clearEvent(event);
    } else if ( (event.what == evKeyDown && event.keyDown.keyCode == kbEsc) ||
                (event.what == evCommand && event.message.command == cmCancel) ) {
        endModal(cmCancel);
        clearEvent(event);
    }
    else
        TListViewer::handleEvent(event);
}

#include "app.h"

void EditorListView::focusItemNum(short item)
{
    if (!disableWrap) {
        if (range > 0) {
            if (item < 0)
                return focusItemNum(range + item);
            else if (item >= range)
                return focusItemNum(item - range);
            focusItem(item);
        }
    } else
        return ListView::focusItemNum(item);
}

void EditorListView::handleEvent(TEvent &ev)
{
    if (ev.what == evCommand)
        switch (ev.message.command) {
            case cmEditorNext:
                focusItemNum(focused + 1);
                clearEvent(ev);
                break;
            case cmEditorPrev:
                focusItemNum(focused - 1);
                clearEvent(ev);
                break;
        }
    else if (ev.what & evMouse) {
        ++disableWrap;
        ListView::handleEvent(ev);
        --disableWrap;
        return;
    }
    ListView::handleEvent(ev);
}
