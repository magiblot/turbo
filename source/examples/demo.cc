#define Uses_TApplication
#define Uses_TDialog
#define Uses_TDeskTop
#define Uses_TButton
#define Uses_TFrame
#define Uses_TScrollBar
#define Uses_TListViewer
#include <tvision/tv.h>
#include <turbo/turbo.h>

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
using turbo::FileEditor;
using EditorList = std::forward_list<FileEditor>;

struct DemoApplication : public TApplication
{
    turbo::SystemClipboard clipboard;

    DemoApplication() noexcept;
};

struct DemoEditorWindow : public TDialog, public turbo::EditorParent
{

    enum { listWidth = 20 };

    EditorList states;
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

    void handleNotification(const SCNotification &, turbo::Editor &) override;

    turbo::TScintilla &createScintilla();
    void addEditor(turbo::TScintilla &, const char *filePath);
    void removeState(FileEditor &aState);
    bool closeAllEditors();

};

struct DemoEditorListView : public TListViewer
{

    DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                     TScrollBar *aVScrollBar, EditorList &aList ) noexcept;

    EditorList &list;

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
    if (edView && edView->editor)
        edView->editor->disassociate();
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
        auto *editor = edView->editor;
        switch (ev.message.command)
        {
            case cmToggleLineNumbers:
                if (editor)
                {
                    editor->lineNumbers.toggle();
                    editor->redraw();
                    clearEvent(ev);
                }
                break;
            case cmToggleLineWrapping:
                if (editor)
                {
                    editor->wrapping.toggle(editor->scintilla);
                    editor->redraw();
                    clearEvent(ev);
                }
                break;
            case cmEditorFocused:
            {
                auto *editor = (FileEditor *) ev.message.infoPtr;
                if (editor)
                {
                    editor->associate(this, edView, leftMargin, hScrollBar, vScrollBar);
                    editor->redraw();
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
                addEditor(createScintilla(), "");
                clearEvent(ev);
                break;
            case cmOpenFile:
            {
                turbo::openFile([&] () -> auto& {
                    return createScintilla();
                }, [&] (turbo::TScintilla &scintilla, const char *path) {
                    addEditor(scintilla, path);
                });
                clearEvent(ev);
                break;
            }
            case cmSaveFile:
                if (editor)
                    ((FileEditor *) editor)->save();
                clearEvent(ev);
                break;
            case cmSaveFileAs:
                if (editor)
                    ((FileEditor *) editor)->saveAs();
                clearEvent(ev);
                break;
            case cmRenameFile:
                if (editor)
                    ((FileEditor *) editor)->rename();
                clearEvent(ev);
                break;
            case cmCloseFile:
                if (editor && ((FileEditor *) editor)->close())
                    removeState(*(FileEditor *) editor);
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
    if (edView && edView->editor)
    {
        auto lastSize = size;
        edView->editor->lockReflow([&] {
            TDialog::dragView(event, mode, limits, minSize, maxSize);
        });
        if (lastSize != size)
            edView->editor->redraw(); // Redraw without reflow lock.
    }
    else
        TDialog::dragView(event, mode, limits, minSize, maxSize);
}

const char *DemoEditorWindow::getTitle(short)
{
    if (edView && edView->editor)
    {
        auto &state = *(turbo::FileEditor *) edView->editor;
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

void DemoEditorWindow::handleNotification(const SCNotification &scn, turbo::Editor &editor)
{
    switch (scn.nmhdr.code)
    {
        case SCN_PAINTED:
            if (!(state & sfDragging) && frame) // It already gets drawn when resizing.
                frame->drawView(); // The frame is sensible to the cursor position and the save point state.
            break;
        case SCN_SAVEPOINTREACHED:
            editor.redraw();
            listView->drawView();
            break;
    }
}

turbo::TScintilla &DemoEditorWindow::createScintilla()
{
    return turbo::createScintilla(clipboard);
}

void DemoEditorWindow::addEditor(turbo::TScintilla &scintilla, const char *filePath)
{
    states.emplace_front(scintilla, filePath);
    listView->setRange(listView->range + 1);
    listView->focusItemNum(0); // Triggers Editor::associate.
    listView->drawView();
}

void DemoEditorWindow::removeState(FileEditor &aState)
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
        while (edView->editor)
        {
            auto &state = *(FileEditor *) edView->editor;
            if (state.close())
                removeState(state);
            else
                return false;
        }
    return true;
}

DemoEditorListView::DemoEditorListView( const TRect& bounds, TScrollBar *aHScrollBar,
                                TScrollBar *aVScrollBar, EditorList &aList ) noexcept :
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
