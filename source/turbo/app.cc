#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TKeys
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TWindow
#define Uses_TFrame
#define Uses_TFileDialog
#define Uses_TIndicator
#define Uses_TStaticText
#define Uses_TParamText
#define Uses_TScreen
#include <tvision/tv.h>

#include <fmt/core.h>
#include <csignal>

#include "app.h"
#include "apputils.h"
#include "editwindow.h"
#include "widgets.h"
#include "listviews.h"
#include "doctree.h"
#include <turbo/fileeditor.h>
#include <turbo/tpath.h>

using namespace Scintilla;
using namespace std::literals;

TurboApp* TurboApp::app = 0;

int main(int argc, const char *argv[])
{
    TurboApp app(argc, argv);
    TurboApp::app = &app;
    app.run();
    app.shutDown();
    TurboApp::app = 0;
}

TurboApp::TurboApp(int argc, const char *argv[]) noexcept :
    TProgInit( &TurboApp::initStatusLine,
               &TurboApp::initMenuBar,
               &TApplication::initDeskTop
             ),
    argc(argc),
    argv(argv)
{
    TCommandSet ts;
    ts += cmSave;
    ts += cmSaveAs;
    ts += cmRename;
    ts += cmOpenRecent;
    ts += cmToggleWrap;
    ts += cmToggleLineNums;
    ts += cmFind;
    ts += cmReplace;
    ts += cmSearchAgain;
    ts += cmSearchPrev;
    ts += cmToggleIndent;
    ts += cmCloseEditor;
    disableCommands(ts);

    // Actions that only make sense when there is at least one editor.
    editorCmds += cmEditorNext;
    editorCmds += cmEditorPrev;
    editorCmds += cmTreeNext;
    editorCmds += cmTreePrev;
    editorCmds += cmCloseAll;
    editorCmds += cmCloseEditor;
    disableCommands(editorCmds);

    // Create the clock view.
    TRect r = getExtent();
    r.a.x = r.b.x - 9;
    r.b.y = r.a.y + 1;
    clock = new TClockView(r);
    clock->growMode = gfGrowLoX | gfGrowHiX;
    insert(clock);

    // Create the document tree view
    {
        TRect r = deskTop->getExtent();
        // Try to make it between 22 and 30 columns wide, and try to leave
        // at least 82 empty columns on screen (so that an editor view is
        // at least ~80 columns by default).
        if (r.b.x > 22)
            r.a.x = r.b.x - std::clamp(r.b.x - 82, 22, 30);
        docTree = new DocumentTreeWindow(r, &docTree);
        docTree->flags &= ~wfZoom;
        // The grow mode assumes it's placed on the right side of the screen.
        // Greater flexibility would require some trick or a dedicated class
        // for side views.
        docTree->growMode = gfGrowLoX | gfGrowHiX | gfGrowHiY;
        docTree->setState(sfShadow, False);
        deskTop->insert(docTree);
        // Show by default only on large terminals.
        if (deskTop->size.x - docTree->size.x < 82)
            docTree->hide();
    }
}

TMenuBar *TurboApp::initMenuBar(TRect r)
{
    r.b.y = r.a.y+1;
    return new TMenuBar( r,
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl-N" ) +
            *new TMenuItem( "~O~pen", cmOpen, kbCtrlO, hcNoContext, "Ctrl-O" ) +
            *new TMenuItem( "Open R~e~cent...", cmOpenRecent, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl-S" ) +
            *new TMenuItem( "S~a~ve As...", cmSaveAs, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~ename...", cmRename, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "~C~lose", cmCloseEditor, kbCtrlW, hcNoContext, "Ctrl-W" ) +
            *new TMenuItem( "Close All", cmCloseAll, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "S~u~spend", cmDosShell, kbNoKey, hcNoContext ) +
            *new TMenuItem( "E~x~it", cmQuit, kbAltX, hcNoContext, "Alt-X" ) +
        *new TSubMenu( "~E~dit", kbAltE ) +
            *new TMenuItem( "~F~ind...", cmFind, kbCtrlF, hcNoContext, "Ctrl-F" ) +
            *new TMenuItem( "~R~eplace...",cmReplace, kbCtrlR, hcNoContext, "Ctrl-R" ) +
            *new TMenuItem( "Find ~N~ext", cmSearchAgain, kbF3, hcNoContext, "F3" ) +
            *new TMenuItem( "Find ~P~revious", cmSearchPrev, kbShiftF3, hcNoContext, "Shift-F3" ) +
        *new TSubMenu( "~W~indows", kbAltW ) +
            *new TMenuItem( "~Z~oom", cmZoom, kbF5, hcNoContext, "F5" ) +
            *new TMenuItem( "~R~esize/move",cmResize, kbCtrlF5, hcNoContext, "Ctrl-F5" ) +
            *new TMenuItem( "~N~ext", cmEditorNext, kbF6, hcNoContext, "F6" ) +
            *new TMenuItem( "~P~revious", cmEditorPrev, kbShiftF6, hcNoContext, "Shift-F6" ) +
            *new TMenuItem( "~C~lose", cmClose, kbAltF3, hcNoContext, "Alt-F3" ) +
            *new TMenuItem( "Previous (in tree)", cmTreePrev, kbAltUp, hcNoContext, "Alt-Up" ) +
            *new TMenuItem( "Next (in tree)", cmTreeNext, kbAltDown, hcNoContext, "Alt-Down" ) +
        *new TSubMenu( "~S~ettings", kbAltS ) +
            *new TMenuItem( "Toggle Line ~N~umbers", cmToggleLineNums, kbF8, hcNoContext, "F8" ) +
            *new TMenuItem( "Toggle Line ~W~rapping", cmToggleWrap, kbF9, hcNoContext, "F9" ) +
            *new TMenuItem( "Toggle Auto ~I~ndent", cmToggleIndent, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Toggle Document ~T~ree View", cmToggleTree, kbNoKey, hcNoContext )
            );

}

TStatusLine *TurboApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Ctrl-N~ New", kbNoKey, cmNew ) +
            *new TStatusItem( "~Ctrl-O~ Open", kbNoKey, cmOpen ) +
            *new TStatusItem( "~Ctrl-S~ Save", kbNoKey, cmSave ) +
            *new TStatusItem( "~F6~ Next", kbF6, cmEditorNext ) +
            *new TStatusItem( "~F12~ Menu" , kbF12, cmMenu ) +
            *new TStatusItem( 0, kbShiftF6, cmEditorPrev ) +
            *new TStatusItem( 0, kbF5, cmZoom ) +
            *new TStatusItem( 0, kbCtrlF5, cmResize )
            );
}

void TurboApp::shutDown()
{
    docTree = nullptr;
    clock = nullptr;
    TApplication::shutDown();
}

void TurboApp::idle()
{
    TApplication::idle();
    if (clock)
        clock->update();
}

void TurboApp::getEvent(TEvent &event)
{
    if (!argsParsed) {
        argsParsed = true;
        parseArgs();
    }
    TApplication::getEvent(event);
    if (event.what == evKeyDown) {
        // Handle key shortcuts that do not have a unique keycode.
        switch (event.keyDown.keyCode)
        {
            case kbTab:
                switch (event.keyDown.controlKeyState & (kbAltShift | kbCtrlShift))
                    case kbCtrlShift:
                    case kbAltShift:
                    {
                        event.what = evCommand;
                        event.message.command = cmEditorNext;
                        event.message.infoPtr = 0;
                    }
                break;
            case kbShiftTab:
                switch (event.keyDown.controlKeyState & (kbAltShift | kbCtrlShift))
                    case kbCtrlShift:
                    case kbAltShift:
                    {
                        event.what = evCommand;
                        event.message.command = cmEditorPrev;
                        event.message.infoPtr = 0;
                    }
                break;
        }
    }
}

void TurboApp::handleEvent(TEvent &event)
{
    TApplication::handleEvent(event);
    bool handled = false;
    if (event.what == evCommand) {
        handled = true;
        switch (event.message.command) {
            case cmNew: fileNew(); break;
            case cmOpen: fileOpen(); break;
            case cmEditorNext:
            case cmEditorPrev:
                showEditorList(&event);
                break;
            case cmCloseAll: closeAll(); break;
            case cmToggleTree: toggleTreeView(); break;
            case cmTreeNext:
                if (docTree)
                    docTree->tree->focusNext();
                break;
            case cmTreePrev:
                if (docTree)
                    docTree->tree->focusPrev();
                break;
            default:
                handled = false;
                break;
        }
    }
    if (handled)
        clearEvent(event);
}

void TurboApp::parseArgs()
{
    if (argc && argv) {
        auto *w = new TWindow(TRect(15, 8, 65, 19), "Please Wait...", wnNoNumber);
        w->flags = 0;
        w->options |= ofCentered;
        w->palette = wpGrayWindow;
        w->insert( new TStaticText(TRect(2, 2, 48, 3), "Opening file:"));
        auto *current = new TParamText(TRect(2, 3, 48, 9));
        w->insert(current);
        insert(w);
        for (int i = 1; i < argc; ++i) {
            char str[256] = {0}; // TParamText is limited to this size.
            strncpy(str, argv[i], 255);
            current->setText(str);
            TScreen::flushScreen();
            fileOpenOrNew(argv[i]);
        }
        remove(w);
        TObject::destroy(w);
    }
}

void TurboApp::fileNew()
{
    addEditor(createScintilla(), "");
}

void TurboApp::fileOpen()
{
    TurboFileDialogs dlgs {*this};
    turbo::openFile([&] () -> auto& {
        return createScintilla();
    }, [&] (auto &scintilla, auto *path) {
        addEditor(scintilla, path);
    }, dlgs);
}

void TurboApp::fileOpenOrNew(const char *path)
{
    char abspath[MAXPATH];
    strnzcpy(abspath, path, MAXPATH);
    fexpand(abspath);
    auto &scintilla = createScintilla();
    turbo::readFile(scintilla, abspath, turbo::silFileDialogs);
    addEditor(scintilla, abspath);
}

void TurboApp::closeAll()
{
    while (!MRUlist.empty()) {
        auto *head = MRUlist.next;
        message((EditorWindow *) head->self, evCommand, cmClose, 0);
        TScreen::flushScreen();
        if (MRUlist.next == head) // Not removed
            break;
    }
}

TRect TurboApp::newEditorBounds() const
{
    if (!MRUlist.empty())
        return MRUlist.next->self->getBounds();
    else {
        TRect r = deskTop->getExtent();
        if (docTree && docTree->state & sfVisible) {
            TRect t = docTree->getBounds();
            // Align left.
            if (t.a.x > r.b.x - t.b.x)
                r.b.x = max(t.a.x, EditorWindow::minSize.x);
            // Align right.
            else
                r.a.x = min(t.b.x, r.b.x - EditorWindow::minSize.x);
        }
        return r;
    }
}

turbo::TScintilla &TurboApp::createScintilla() noexcept
{
    return turbo::createScintilla(&clipboard);
}

void TurboApp::addEditor(turbo::TScintilla &scintilla, const char *path)
// Pre: 'path' is an absolute path.
{
    TRect r = newEditorBounds();
    auto &counter = fileCount[TPath::basename(path)];
    EditorWindow &w = *new EditorWindow(r, scintilla, path, counter, *this);
    updatePalette(w);
    if (docTree)
        docTree->tree->addEditor(&w);
    w.listHead.insert_after(&MRUlist);
    deskTop->insert(&w);
    enableCommands(editorCmds);
}

void TurboApp::showEditorList(TEvent *ev)
{
    EditorList list {&MRUlist};
    TRect r {0, 0, 0, 0};
    r.b.x = std::clamp<int>(list.measureWidth() + 6, 40, deskTop->size.x - 10);
    r.b.y = std::clamp<int>(list.size() + 2, 6, deskTop->size.y - 4);
    r.move((deskTop->size.x - r.b.x) / 2,
           (deskTop->size.y - r.b.y) / 4);
    ListWindow *lw = new ListWindow(r, "Buffer List", list, &initViewer<EditorListView>);
    if (ev)
        lw->putEvent(*ev);
    if (deskTop->execView(lw) == cmOK) {
        auto *head = (list_head<EditorWindow> *) lw->getSelected();
        head->self->focus();
    }

    destroy(lw);
}

void TurboApp::toggleTreeView()
{
    if (!docTree)
        return;
    // Prevent editors from doing draw() on each changeBounds(). We'll draw all
    // the views at the end.
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, False);
    });
    TRect dr = docTree->getBounds();
    if (docTree->state & sfVisible) {
        docTree->hide();
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x >= dr.b.x)
                r.a.x -= docTree->size.x;
            else if (r.b.x <= dr.a.x)
                r.b.x += docTree->size.x;
            win->locate(r);
        });
    } else {
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x + docTree->size.x >= dr.b.x)
                r.a.x += docTree->size.x;
            else if (r.b.x - docTree->size.x <= dr.a.x)
                r.b.x -= docTree->size.x;
            win->locate(r);
        });
        docTree->show();
    }
    MRUlist.forEach([&] (auto *win) {
        win->setState(sfExposed, True);
    });
    deskTop->redraw();
}

void TurboApp::handleFocus(EditorWindow &w) noexcept
{
    // w has been focused, so it becomes the first of our MRU list.
    w.listHead.insert_after(&MRUlist);
    if (docTree)
        docTree->tree->focusEditor(&w);
    // We keep track of the most recent directory for file dialogs.
    if (!w.filePath().empty())
        mostRecentDir = TPath::dirname(w.filePath());
}

void TurboApp::handleTitleChange(EditorWindow &w) noexcept
{
    auto &counter = fileCount[TPath::basename(w.filePath())];
    if (&counter != w.fileNumber.counter)
    {
        w.fileNumber = {counter};
        if (docTree)
        {
            docTree->tree->removeEditor(&w);
            docTree->tree->addEditor(&w);
            docTree->tree->focusEditor(&w);
        }
    }
    if (!w.filePath().empty() && w.state & sfActive)
        mostRecentDir = TPath::dirname(w.filePath());
}

void TurboApp::removeEditor(EditorWindow &w) noexcept
{
    w.listHead.remove();
    if (MRUlist.empty())
        disableCommands(editorCmds);
    if (docTree)
    {
        docTree->tree->removeEditor(&w);
        // Removing the editor causes the focus to stay on the same position
        // but maybe not on the right element.
        if (!MRUlist.empty())
            docTree->tree->focusEditor(MRUlist.next->self);
    }
}

const char *TurboApp::getFileDialogDir() noexcept
{
    return mostRecentDir.c_str();
}

static constexpr TColorAttr cpTurboAppColor[167] =
{
    /* 1 = TBackground, 2-7 = TMenuView and TStatusLine */
          0x71, 0x70, 0x78, 0x74, 0x20, 0x28, 0x24,
    /* 8-15 = TWindow(Blue) */
    0x17, 0x1F, 0x1A, 0x31, 0x31, 0x1E, 0x71, 0x1F,
    /* 16-23 = TWindow(Cyan) */
    0x37, 0x3F, 0x3A, 0x13, 0x13, 0x3E, 0x21, 0x3F,
    /* 24-31 = TWindow(Gray) */
    0x70, 0x7F, 0x7A, 0x13, 0x13, 0x70, 0x7F, 0x7E,
    /* 32-63 = TDialog(Gray) */
    0x70, 0x7F, 0x7A, 0x13, 0x13, 0x70, 0x70, 0x7F,
    0x7E, 0x20, 0x2B, 0x2F, 0x78, 0x2E, 0x70, 0x30,
    0x3F, 0x3E, 0x1F, 0x2F, 0x1A, 0x20, 0x72, 0x31,
    0x31, 0x30, 0x2F, 0x3E, 0x31, 0x13, 0x38, 0x00,
    /* 64-95 = TDialog(Blue) */
    0x17, 0x1F, 0x1A, 0x71, 0x71, 0x1E, 0x17, 0x1F,
    0x1E, 0x20, 0x2B, 0x2F, 0x78, 0x2E, 0x10, 0x30,
    0x3F, 0x3E, 0x70, 0x2F, 0x7A, 0x20, 0x12, 0x31,
    0x31, 0x30, 0x2F, 0x3E, 0x31, 0x13, 0x38, 0x00,
    /* 96-127 = TDialog(Cyan) */
    0x37, 0x3F, 0x3A, 0x13, 0x13, 0x3E, 0x30, 0x3F,
    0x3E, 0x20, 0x2B, 0x2F, 0x78, 0x2E, 0x30, 0x70,
    0x7F, 0x7E, 0x1F, 0x2F, 0x1A, 0x20, 0x32, 0x31,
    0x71, 0x70, 0x2F, 0x7E, 0x71, 0x13, 0x38, 0x00,
    /* 128-135 = Help Colors? */
    0x37, 0x3F, 0x3A, 0x13, 0x13, 0x30, 0x3E, 0x1E,
    /* 136-167 = EditorWindow */
    0x07, 0x0F, 0x0A, 0x30, 0x30, 0x0F, 0x08, 0x0F,
    0x06, 0x20, 0x2B, 0x2F, 0x78, 0x2E, 0x08, 0x30,
    0x3F, 0x3E, 0x1F, 0x2F, 0x1A, 0x20, 0x72, 0x31,
    0x31, 0x30, 0x2F, 0x3E, 0x31, 0x13, 0x38, 0x00,
};

static_assert(edReserved == 167, "");

TPalette& TurboApp::getPalette() const
{
    static TPalette palette(cpTurboAppColor);
    return palette;
}

void TurboApp::updatePalette(EditorWindow &w) const
{
    using namespace turbo;
    auto &pal = getPalette();
    auto &schema = w.editor.theming.getSchema();
    pal[edFramePassive  ] = normalize(schema, sFramePassive  );
    pal[edFrameActive   ] = normalize(schema, sFrameActive   );
    pal[edFrameIcon     ] = normalize(schema, sFrameIcon     );
    pal[edStaticText    ] = normalize(schema, sStaticText    );
    pal[edLabelNormal   ] = normalize(schema, sLabelNormal   );
    pal[edLabelSelected ] = normalize(schema, sLabelSelected );
    pal[edLabelShortcut ] = normalize(schema, sLabelShortcut );
    pal[edButtonNormal  ] = normalize(schema, sButtonNormal  );
    pal[edButtonDefault ] = normalize(schema, sButtonDefault );
    pal[edButtonSelected] = normalize(schema, sButtonSelected);
    pal[edButtonDisabled] = normalize(schema, sButtonDisabled);
    pal[edButtonShortcut] = normalize(schema, sButtonShortcut);
    pal[edButtonShadow  ] = normalize(schema, sButtonShadow  );
}
