#define Uses_TApplication
#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TButton
#define Uses_TFrame
#define Uses_TScrollBar
#define Uses_TListViewer
#include <tvision/tv.h>
#include <turbo/turbo.h>
#include <turbo/clipboard.h>
#include <turbo/tpath.h>

#include <memory>
#include <utility>
#include <forward_list>
#include <vector>

enum : ushort
{
    cmToggleLineNumbers = 1000,
    cmToggleLineWrapping,
    cmEditorFocused,
    cmNewFile,
    cmOpenFile,
    cmSaveFile,
    cmSaveFileAs,
    cmRenameFile,
    cmCloseFile,
};

struct DemoEditorListView;
using turbo::FileEditorState;
using EditorStateList = std::forward_list<FileEditorState>;

struct DemoApplication : public TApplication
{
    turbo::LcbClipboard clipboard;

    DemoApplication() noexcept;
};

struct DemoEditorWindow : public TDialog, public turbo::EditorParent
{

    enum { listWidth = 20 };

    EditorStateList states;
    turbo::EditorView *edView;
    turbo::LeftMarginView *leftMargin;
    TScrollBar *hScrollBar, *vScrollBar;
    DemoEditorListView *listView;
    std::vector<char> title;
    turbo::Clipboard *clipboard;

    DemoEditorWindow(const TRect &bounds, turbo::Clipboard *aClipboard) noexcept;

    void shutDown() override;
    void handleEvent(TEvent &ev) override;
    Boolean valid(ushort command) override;
    void dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize) override;
    const char *getTitle(short) override;

    void handleNotification(ushort, turbo::EditorState &) noexcept override;

    turbo::Editor &createEditor();
    void addEditor(turbo::Editor &, const char *filePath);
    void removeState(FileEditorState &aState);
    bool closeAllEditors();

};

struct DemoEditorListView : public TListViewer
{

    DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                     TScrollBar *aVScrollBar, EditorStateList &aList ) noexcept;

    EditorStateList &list;

    void getText(char *dest, short item, short maxLen) override;
    void focusItemNum(short item) override;
    void setRange(short aRange);

    int maxWidth();

};

DemoApplication::DemoApplication() noexcept :
    TProgInit(&initStatusLine,
              &initMenuBar,
              &initDeskTop)
{
    insertWindow(
        new DemoEditorWindow(
            deskTop->getExtent().grow(-2, -2),
            &clipboard
        )
    );
}

DemoEditorWindow::DemoEditorWindow(const TRect &bounds, turbo::Clipboard *aClipboard) noexcept :
    TWindowInit(&initFrame),
    TDialog(bounds, nullptr),
    clipboard(aClipboard)
{
    using namespace turbo;
    flags |= wfGrow;

    TRect viewBounds = getExtent().grow(-2, -4).move(0, -2);
    viewBounds.b.x -= listWidth + 1;

    edView = new EditorView(viewBounds);
    leftMargin = new LeftMarginView(1);
    hScrollBar = [&] {
        TRect r = viewBounds;
        r.a.y = r.b.y;
        r.b.y += 1;
        return new TScrollBar(r);
    }();
    vScrollBar = [&] {
        TRect r = viewBounds;
        r.a.x = r.b.x;
        r.b.x += 1;
        return new TScrollBar(r);
    }();
    TPoint butOrigin = {viewBounds.a.x, viewBounds.b.y + 2};
    auto addButton = [&] (ushort command, TStringView text) {
        TRect r = TRect(butOrigin, butOrigin + TPoint {cstrlen(text) + 4, 2});
        auto *but = new TButton(r, text, command, bfNormal);
        but->growMode = gfGrowLoY | gfGrowHiY;
        insert(but);
        butOrigin = {r.b.x + 1, r.a.y};
    };
    addButton(cmToggleLineNumbers, "Line Numbers");
    addButton(cmToggleLineWrapping, "Line Wrapping");
    addButton(cmNewFile, "New File");
    addButton(cmOpenFile, "Open File");
    addButton(cmSaveFile, "Save");
    addButton(cmSaveFileAs, "Save As");
    addButton(cmRenameFile, "Rename");
    addButton(cmCloseFile, "Close");
    listView = [&] {
        TRect r = viewBounds;
        r.a.x = r.b.x + 1;
        r.b.x += listWidth - 1;
        r.move(1, 0);
        auto *hScrollBar = [&] {
            TRect s = r;
            s.a.y = s.b.y;
            s.b.y += 1;
            return new TScrollBar(s);
        }();
        auto *vScrollBar = [&] {
            TRect s = r;
            s.a.x = s.b.x;
            s.b.x += 1;
            return new TScrollBar(s);
        }();
        hScrollBar->growMode |= gfGrowLoX;
        insert(hScrollBar);
        insert(vScrollBar);
        return new DemoEditorListView(r, hScrollBar, vScrollBar, states);
    }();
    listView->growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
    // The editor view needs to go first so that, when resizing the window, the other
    // views get resized before it, which takes care of redrawing them all.
    insert(edView);
    insert(leftMargin);
    insert(hScrollBar);
    insert(vScrollBar);
    insert(listView);
}

void DemoEditorWindow::shutDown()
{
    if (edView && edView->editorState)
        edView->editorState->disassociate();
    edView = nullptr;
    leftMargin = nullptr;
    hScrollBar = nullptr;
    vScrollBar = nullptr;
    listView = nullptr;
    TDialog::shutDown();
}

void DemoEditorWindow::handleEvent(TEvent &ev)
{
    if (ev.what == evCommand && edView)
    {
        switch (ev.message.command)
        {
            case cmToggleLineNumbers:
                if (edView->editorState)
                {
                    edView->editorState->toggleLineNumbers();
                    edView->editorState->redraw();
                    clearEvent(ev);
                }
                break;
            case cmToggleLineWrapping:
                if (edView->editorState)
                {
                    edView->editorState->toggleLineWrapping();
                    edView->editorState->redraw();
                    clearEvent(ev);
                }
                break;
            case cmEditorFocused:
            {
                auto *state = (FileEditorState *) ev.message.infoPtr;
                if (state)
                {
                    state->associate(this, edView, leftMargin, hScrollBar, vScrollBar);
                    state->redraw();
                }
                else
                {
                    edView->drawView();
                    frame->drawView();
                }
                clearEvent(ev);
                break;
            }
            case cmNewFile:
                addEditor(createEditor(), "");
                clearEvent(ev);
                break;
            case cmOpenFile:
            {
                turbo::openFile([&] () -> auto& {
                    return createEditor();
                }, [&] (turbo::Editor &editor, const char *path) {
                    addEditor(editor, path);
                });
                clearEvent(ev);
                break;
            }
            case cmSaveFile:
                if (edView->editorState)
                    ((FileEditorState *) edView->editorState)->save();
                clearEvent(ev);
                break;
            case cmSaveFileAs:
                if (edView->editorState)
                    ((FileEditorState *) edView->editorState)->saveAs();
                clearEvent(ev);
                break;
            case cmRenameFile:
                if (edView->editorState)
                    ((FileEditorState *) edView->editorState)->rename();
                clearEvent(ev);
                break;
            case cmCloseFile:
                if (edView->editorState && ((FileEditorState *) edView->editorState)->close())
                    removeState(*(FileEditorState *) edView->editorState);
                clearEvent(ev);
                break;
        }
    }
    TDialog::handleEvent(ev);
}

Boolean DemoEditorWindow::valid(ushort command)
{
    if (TDialog::valid(command))
        switch (command)
        {
            case cmQuit:
            case cmClose:
                return closeAllEditors();
            default:
                return true;
        }
    return false;
}

void DemoEditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    if (edView && edView->editorState)
    {
        auto lastResizeLock = edView->editorState->resizeLock;
        auto lastSize = edView->size;
        edView->editorState->resizeLock = true;
        TDialog::dragView(event, mode, limits, minSize, maxSize);
        edView->editorState->resizeLock = lastResizeLock;
        if (lastSize != edView->size)
            edView->editorState->redraw(); // Redraw without 'resizeLock = true'.
    }
    else
        TDialog::dragView(event, mode, limits, minSize, maxSize);
}

const char *DemoEditorWindow::getTitle(short)
{
    if (edView && edView->editorState)
    {
        auto &state = *(turbo::FileEditorState *) edView->editorState;
        auto name = TPath::basename(state.filePath);
        if (name.empty()) name = "Untitled";
        bool dirty = !state.inSavePoint();
        size_t length = name.size() + dirty;
        title.resize(0);
        title.resize(length + 1);
        memcpy(title.data(), name.data(), name.size());
        if (dirty) title[name.size()] = '*';
        title[length] = '\0';
        return title.data();
    }
    return nullptr;
}

void DemoEditorWindow::handleNotification(ushort code, turbo::EditorState &state) noexcept
{
    using namespace turbo;
    switch (code)
    {
        case EditorState::ncPainted:
            if (!state.resizeLock) // These already get drawn when resizing.
                frame->drawView(); // The frame is sensible to the save point state.
            break;
        case FileEditorState::ncSaved:
            state.redraw();
            listView->drawView();
            break;
    }
}

turbo::Editor &DemoEditorWindow::createEditor()
{
    return turbo::createEditor(clipboard);
}

void DemoEditorWindow::addEditor(turbo::Editor &editor, const char *filePath)
{
    states.emplace_front(editor, filePath);
    listView->setRange(listView->range + 1);
    listView->focusItemNum(0); // Triggers EditorState::associate.
    listView->drawView();
}

void DemoEditorWindow::removeState(FileEditorState &aState)
// Pre: 'aState' belongs to 'states'.
{
    states.remove_if([&] (const auto &state) {
        if (&state == &aState)
            return aState.disassociate(), true;
        return false;
    });
    // 'aState' is dangling by this point.
    listView->setRange(listView->range - 1);
    listView->focusItemNum(listView->focused);
    listView->drawView();
}

bool DemoEditorWindow::closeAllEditors()
{
    if (edView)
        while (edView->editorState)
        {
            auto &state = *(FileEditorState *) edView->editorState;
            if (state.close())
                removeState(state);
            else
                return false;
        }
    return true;
}

DemoEditorListView::DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar, EditorStateList &aList ) noexcept :
    TListViewer(bounds, 1, aHScrollBar, aVScrollBar),
    list(aList)
{
}

void DemoEditorListView::getText(char *dest, short item, short maxChars)
{
    short i = 0;
    for (auto &state : list)
        if (i++ == item)
        {
            if (!state.filePath.empty())
                strnzcpy(dest, state.filePath, maxChars + 1);
            else
                strnzcpy(dest, "Untitled", maxChars + 1);
            return;
        }
    snprintf(dest, maxChars, "<ERROR: out-of-bounds index %hd>", item);
}

void DemoEditorListView::focusItemNum(short item)
{
    TListViewer::focusItemNum(item);
    short i = 0;
    for (auto &state : list)
        if (i++ == focused)
        {
            message(owner, evCommand, cmEditorFocused, &state);
            return;
        }
    message(owner, evCommand, cmEditorFocused, nullptr);
}

void DemoEditorListView::setRange(short aRange)
{
    TListViewer::setRange(aRange);
    if (hScrollBar != 0)
        hScrollBar->setRange(0, maxWidth() - size.x + 2);
}

int DemoEditorListView::maxWidth()
{
    char text[256];
    int width = 0;
    for (short i = 0; i < range; ++i)
    {
        getText(text, i, sizeof(text) - 1);
        width = max(strwidth(text), width);
    }
    return width;
}

int main()
{
    DemoApplication app;
    app.run();
    app.shutDown();
}
