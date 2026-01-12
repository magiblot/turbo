#define Uses_TKeys
#define Uses_TEvent
#define Uses_TScrollBar
#include <tvision/tv.h>

#include "listviews.h"
#include <turbo/tpath.h>

/////////////////////////////////////////////////////////////////////////
// ListModel

size_t ListModel::maxItemCStrLen(const ListModel &model) noexcept
{
    size_t maxWidth = 0, elems = model.size();
    for (size_t i = 0; i < elems; ++i)
    {
        std::string text = model.getText(model.at(i));
        size_t width = (size_t) cstrlen(text);
        if (width > maxWidth)
            maxWidth = width;
    }
    return maxWidth;
}

/////////////////////////////////////////////////////////////////////////
// ListWindow

const TPoint ListWindow::minSize = {5, 3};

ListWindow::ListWindow( const TRect &bounds, TStringView aTitle, const ListModel &model,
                        ushort listViewFlags, ListViewCreator createListView ) noexcept :
    TWindowInit(&initFrame),
    TWindow(bounds, aTitle, wnNoNumber)
{
    flags = wfClose | wfMove;

    TScrollBar *hScrollBar = nullptr, *vScrollBar = nullptr;
    if (listViewFlags & lvScrollBars)
    {
        hScrollBar = standardScrollBar(sbHorizontal | sbHandleKeyboard);
        vScrollBar = standardScrollBar(sbVertical | sbHandleKeyboard);
    }
    listView = &createListView( getExtent().grow(-1, -1), hScrollBar,
                                vScrollBar, model, listViewFlags );
    listView->growMode = gfGrowHiX | gfGrowHiY;
    insert(listView);
}

void ListWindow::shutDown()
{
    listView = nullptr;
    TWindow::shutDown();
}

TColorAttr ListWindow::mapColor(uchar index)
{
    switch (index)
    {
        case 1: return '\x1F';
        case 2: return '\x1F';
        case 3: return '\x1A';
        case 4: return '\x31';
        case 5: return '\x72';
        case 6: return '\x1F';
        case 7: return '\x2F';
        case 8: return '\x1B';
        case 9: return '\x2B';
        default: return errorAttr;
    }
}

void* ListWindow::getCurrent() const noexcept
{
    if (listView)
        return listView->getCurrent();
    return nullptr;
}

short ListWindow::getCurrentIndex() const noexcept
{
    if (listView)
        return listView->focused;
    return 0;
}

void ListWindow::setCurrentIndex(short i) noexcept
{
    if (listView)
        listView->focusItemNum(i);
}

void ListWindow::handleEvent(TEvent& event)
{
    TWindow::handleEvent(event);
    if (event.what == evMouseDown && !mouseInView(event.mouse.where))
    {
        endModal(cmCancel);
        clearEvent(event);
    }
}

void ListWindow::sizeLimits(TPoint &min, TPoint &max)
{
    TView::sizeLimits(min, max);
    min = minSize;
}

/////////////////////////////////////////////////////////////////////////
// ListView

#define cpListView "\x06\x07\x08\x09"

ListView::ListView( const TRect& bounds, TScrollBar *aHScrollBar,
                    TScrollBar *aVScrollBar, const ListModel &aModel,
                    ushort aFlags ) :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    model(aModel),
    flags(aFlags)
{
    setRange(model.size());
    if (range > 1)
        focusItem(1);
    if (hScrollBar)
        hScrollBar->setRange(0, ListModel::maxItemCStrLen(model) - size.x + 2);
}

TPalette &ListView::getPalette() const
{
    static TPalette palette(cpListView, sizeof(cpListView) - 1);
    return palette;
}

void *ListView::getCurrent() noexcept
{
    return model.at(focused);
}

void ListView::handleEvent(TEvent &event)
{
    if (event.what == evMouseDown)
    {
        int mouseAutosToSkip = 4;
        int newItem = focused;
        int oldItem = focused;
        int count = 0;
        do
        {
            TPoint mouse = makeLocal(event.mouse.where);
            if (range <= size.y || (0 <= mouse.y && mouse.y < size.y))
                newItem = mouse.y + topItem;
            else
            {
                if (event.what == evMouseAuto)
                    count++;
                if (count == mouseAutosToSkip)
                {
                    count = 0;
                    if (mouse.y < 0)
                        newItem = focused - 1;
                    else if (mouse.y >= size.y)
                        newItem = focused + 1;
                }
            }
            if (newItem != oldItem)
            {
                focusItemNum(newItem);
                drawView();
            }
            oldItem = newItem;
            if (event.mouse.eventFlags & meDoubleClick)
                break;
        }
        while (mouseEvent(event, evMouseMove | evMouseAuto));
        focusItemNum(newItem);
        drawView();
        if ( ((event.mouse.eventFlags & meDoubleClick) || (flags & lvSelectSingleClick)) &&
             0 <= newItem && newItem < range )
            endModal(cmOK);
        clearEvent(event);
    }
    else if (event.what == evKeyDown && event.keyDown.keyCode == kbEnter)
    {
        endModal(cmOK);
        clearEvent(event);
    }
    else if ( (event.what == evKeyDown && event.keyDown.keyCode == kbEsc) ||
              (event.what == evCommand && event.message.command == cmCancel) )
    {
        endModal(cmCancel);
        clearEvent(event);
    }
    else
        TListViewer::handleEvent(event);
}

void ListView::draw()
{
    TDrawBuffer b;

    TAttrPair normalColors = getColor(0x0301);
    TAttrPair focusedColors = getColor(0x0402);

    int indent = (hScrollBar) ? hScrollBar->value : 0;

    bool focusedVis = false;
    for (int i = 0; i < size.y; ++i)
    {
        int item = i + topItem;

        TAttrPair colors = normalColors;
        uchar scOff = 4;
        if (focused == item && range > 0)
        {
            colors = focusedColors;
            setCursor(1, i);
            scOff = 0;
            focusedVis = true;
        }

        b.moveChar(0, ' ', colors, size.x);
        if (item < range)
        {
            std::string text = model.getText(model.at(item));
            text.insert(text.begin(), ' ');
            b.moveCStr(0, text, colors, size.x, indent);

            if (showMarkers)
            {
                b.putChar(0, specialChars[scOff] );
                b.putChar(size.x - 1, specialChars[scOff + 1]);
            }
        }
        else if (i == 0)
            b.moveStr(1, "<empty>", getColor(1));

        writeLine(0, i, size.x, 1, b);
    }

    if (!focusedVis)
        setCursor(-1, -1);
}

/////////////////////////////////////////////////////////////////////////
// EditorListView

#include "cmds.h"

static int mod(int a, int b)
{
    int m = a % b;
    if (m < 0)
        return b < 0 ? m - b : m + b;
    return m;
}

void EditorListView::handleEvent(TEvent &ev)
{
    if (ev.what == evCommand && ev.message.command == cmEditorNext)
    {
        focusItemNum(mod(focused + 1, range));
        clearEvent(ev);
    }
    else if (ev.what == evCommand && ev.message.command == cmEditorPrev)
    {
        focusItemNum(mod(focused - 1, range));
        clearEvent(ev);
    }
    else
        ListView::handleEvent(ev);
}

/////////////////////////////////////////////////////////////////////////
// EditorListModel

size_t EditorListModel::size() const noexcept
{
    return list.size();
}

void *EditorListModel::at(size_t i) const noexcept
{
    return (i < list.size()) ? list.at(i)->self : nullptr;
}

std::string EditorListModel::getText(void *item) const noexcept
{
    std::string text;
    if (auto *wnd = (EditorWindow *) item)
    {
        text.append(wnd->title);
        if (!wnd->filePath().empty())
        {
            text.append("  ~");
            text.append(TPath::dirname(wnd->filePath()));
            text.append("~");
        }
    }
    return text;
}
