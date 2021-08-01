#define Uses_TApplication
#define Uses_TDialog
#define Uses_TScrollBar
#include <tvision/tv.h>

#include <turbo/basicwindow.h>
#include <turbo/basicframe.h>

namespace turbo {

TFrame* BasicEditorWindow::initFrame(TRect bounds)
{
    return new BasicEditorFrame(bounds);
}

BasicEditorWindow::BasicEditorWindow(const TRect &bounds, Editor &aEditor) :
    TWindowInit(&initFrame),
    TWindow(bounds, nullptr, wnNoNumber),
    editor(aEditor)
{
    ((BasicEditorFrame *) frame)->scintilla = &aEditor.scintilla;
    options |= ofTileable | ofFirstClick;
    setState(sfShadow, False);

    auto *editorView = new EditorView(TRect(1, 1, size.x - 1, size.y - 1));
    insert(editorView);

    auto *leftMargin = new LeftMarginView(leftMarginSep);
    leftMargin->options |= ofFramed;
    insert(leftMargin);

    auto *hScrollBar = new TScrollBar(TRect(18, size.y - 1, size.x - 2, size.y));
    hScrollBar->hide();
    insert(hScrollBar);

    auto *vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar->hide();
    insert(vScrollBar);

    editor.associate(this, editorView, leftMargin, hScrollBar, vScrollBar);
}

BasicEditorWindow::~BasicEditorWindow()
{
    delete &editor;
}

void BasicEditorWindow::shutDown()
{
    editor.disassociate();
    TWindow::shutDown();
}

void BasicEditorWindow::setState(ushort aState, Boolean enable)
{
    TWindow::setState(aState, enable);
    if (aState == sfActive && editor.parent == this)
    {
        editor.hScrollBar->setState(sfVisible, enable);
        editor.vScrollBar->setState(sfVisible, enable);
    }
}

void BasicEditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    auto lastResizeLock = editor.resizeLock;
    auto lastSize = size;
    editor.resizeLock = true;
    TWindow::dragView(event, mode, limits, minSize, maxSize);
    editor.resizeLock = lastResizeLock;
    if (lastSize != size)
        editor.redraw(); // Redraw without 'resizeLock = true'.
}

void BasicEditorWindow::sizeLimits(TPoint &min, TPoint &max)
{
    TView::sizeLimits(min, max);
    min = minSize;
}

TColorAttr BasicEditorWindow::mapColor(uchar index) noexcept
{
    if (0 < index && index - 1 < WindowPaletteItemCount)
        return getScheme().palette[index - 1];
    return errorAttr;
}

void BasicEditorWindow::handleNotification(ushort code, Editor &editor) noexcept
{
    if (code == Editor::ncPainted)
        if (!editor.resizeLock && frame) // It already gets drawn when resizing.
            frame->drawView(); // The frame is sensible to the cursor position and the save point state.
}

#define dialogColor(i) cpAppColor[(uchar) (cpDialog[i - 1] - 1)]

extern constexpr WindowColorScheme windowSchemeDefault =
{
    schemaDefault,
    {
        /* wndFramePassive             */ '\x07',
        /* wndFrameActive              */ '\x0F',
        /* wndFrameIcon                */ '\x0A',
        /* wndScrollBarPageArea        */ '\x30',
        /* wndScrollBarControls        */ '\x30',
        /* wndStaticText               */ '\x0F',
        /* wndLabelNormal              */ '\x08',
        /* wndLabelSelected            */ '\x0F',
        /* wndLabelShortcut            */ '\x06',
        /* wndButtonNormal             */ '\x20',
        /* wndButtonDefault            */ '\x2B',
        /* wndButtonSelected           */ '\x2F',
        /* wndButtonDisabled           */ '\x78',
        /* wndButtonShortcut           */ '\x2E',
        /* wndButtonShadow             */ '\x08',
        /* wndClusterNormal            */ dialogColor(16),
        /* wndClusterSelected          */ dialogColor(17),
        /* wndClusterShortcut          */ dialogColor(18),
        /* wndInputLineNormal          */ dialogColor(19),
        /* wndInputLineSelected        */ dialogColor(20),
        /* wndInputLineArrows          */ dialogColor(21),
        /* wndHistoryArrow             */ dialogColor(22),
        /* wndHistorySides             */ dialogColor(23),
        /* wndHistWinScrollBarPageArea */ dialogColor(24),
        /* wndHistWinScrollBarControls */ dialogColor(25),
        /* wndListViewerNormal         */ dialogColor(26),
        /* wndListViewerFocused        */ dialogColor(27),
        /* wndListViewerSelected       */ dialogColor(28),
        /* wndListViewerDivider        */ dialogColor(29),
        /* wndInfoPane                 */ dialogColor(30),
        /* wndClusterDisabled          */ dialogColor(31),
    }
};

#undef dialogColor

} // namespace turbo
