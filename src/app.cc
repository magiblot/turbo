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
#include "editwindow.h"
#include "util.h"
#include "widgets.h"
#include "listviews.h"
#include "doctree.h"

using namespace Scintilla;
using namespace std::literals;

TurboApp* TurboApp::app = 0;

TurboApp::TurboApp(int argc, const char *argv[]) :
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
            *new TMenuItem( "Open ~R~ecent...", cmOpenRecent, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl-S" ) +
            *new TMenuItem( "S~a~ve As...", cmSaveAs, kbNoKey, hcNoContext ) +
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
            *new TStatusItem( 0, kbShiftF6, cmEditorPrev )
            );
}

void TurboApp::idle()
{
    TApplication::idle();
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
        switch (event.keyDown.keyCode) {
            case kbTab:
                switch (event.keyDown.controlKeyState & (kbShift | kbAltShift | kbCtrlShift)) {
                    case kbAltShift:
                        event.what = evCommand;
                        event.message.command = cmEditorNext;
                        event.message.infoPtr = 0;
                        break;
                    case kbShift | kbAltShift:
                        event.what = evCommand;
                        event.message.command = cmEditorPrev;
                        event.message.infoPtr = 0;
                        break;
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
            case cmDosShell: shell(); break;
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

void TurboApp::shell()
{
    suspend();
#ifndef __unix__
    cout << "Type EXIT to return..." << endl;
    system( getenv( "COMSPEC"));
#else
    cout << "The application has been stopped. You can return by entering 'fg'." << endl;
    raise(SIGTSTP);
#endif
    resume();
    redraw();
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
            openEditor(argv[i], true);
        }
        remove(w);
        TObject::destroy(w);
    }
}

void TurboApp::fileNew()
{
    openEditor({});
}

void TurboApp::fileOpen()
{
    openFileDialog( "*.*", "Open file", "~N~ame", fdOpenButton, 0,
        [this] (TView *dialog) {
            // MAXPATH as assumed by TFileDialog.
            char fileName[MAXPATH];
            dialog->getData(fileName);
            return openEditor(fileName);
        }
    );
}

bool TurboApp::openEditor(std::string_view fileName, bool canFail)
{
    TRect r = newEditorBounds();
    EditorWindow *w = new EditorWindow(r, fileName, canFail);
    w = (EditorWindow *) validView(w);
    if (w)
        addEditor(w);
    return w;
}

void TurboApp::closeAll()
{
    auto *head = MRUlist.next;
    while (head != &MRUlist) {
        auto *next = head->next;
        message((EditorWindow *) head->self, evCommand, cmClose, 0);
        TScreen::flushScreen();
        if (next->prev == head) // Not removed
            break;
        head = next;
    }
}

TRect TurboApp::newEditorBounds() const
{
    if (!MRUlist.empty())
        return MRUlist.next->self->getBounds();
    else {
        TRect r = deskTop->getExtent();
        if (docTree->state & sfVisible) {
            TRect t = docTree->getBounds();
            // Align left.
            if (t.a.x > r.b.x - t.b.x)
                r.b.x = max(t.a.x, EditorWindow::minEditWinSize.x);
            // Align right.
            else
                r.a.x = min(t.b.x, r.b.x - EditorWindow::minEditWinSize.x);
        }
        return r;
    }
}

void TurboApp::setEditorTitle(EditorWindow *w)
{
    uint number;
    auto &&file = w->file.filename();
    if (!file.empty()) {
        w->title.assign(file);
        number = ++getFileCounter(file.native());
    } else {
        w->title.assign("Untitled"s);
        number = ++fileCount[{}];
    }
    if (number > 1)
        w->title.append(fmt::format(" ({})", number));
    w->name = w->title; // Copy!
}

void TurboApp::updateEditorTitle(EditorWindow *w, const util::u8path &prevFile)
{
    --getFileCounter(prevFile.filename().native());
    setEditorTitle(w);
    if (docTree) {
        docTree->tree->removeEditor(w);
        docTree->tree->addEditor(w);
        docTree->tree->focusEditor(w);
    }
    if (!w->file.empty())
        mostRecentDir.assign(w->file.parent_path());
}

active_counter& TurboApp::getFileCounter(std::string_view file)
{
    // We need to keep at least one copy of the 'file' string alive.
    // This is because I don't want fileCount to be a map of std::string.
    auto it = fileCount.find(file);
    if (it == fileCount.end()) {
        // Allocate string for the filename.
        const auto &s = files.emplace_back(file);
        file = s; // Make file point to the allocated string.
        it = fileCount.emplace(file, active_counter()).first;
    }
    return it->second;
}

void TurboApp::addEditor(EditorWindow *w)
{
    setEditorTitle(w);
    if (docTree)
        docTree->tree->addEditor(w);
    w->MRUhead.insert_after(&MRUlist);
    deskTop->insert(w);
    enableCommands(editorCmds);
}

void TurboApp::removeEditor(EditorWindow *w)
{
    --getFileCounter(w->file.native());
    w->MRUhead.remove();
    if (MRUlist.empty())
        disableCommands(editorCmds);
    if (docTree) {
        docTree->tree->removeEditor(w);
        // We need to set the focus again as it had already been set before
        // removing the editor, and so it would stay on the same position
        // but not on the same element.
        if (!MRUlist.empty())
            docTree->tree->focusEditor(MRUlist.next->self);
    }
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
    MRUlist.forEach([] (auto *win) {
        // Set exposed=False to prevent Turbo Vision from attempting
        // to draw all views, which can lead to polinomial complexity.
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
        docTree->show();
        MRUlist.forEach([this, dr] (auto *win) {
            TRect r = win->getBounds();
            if (r.a.x + docTree->size.x >= dr.b.x)
                r.a.x += docTree->size.x;
            else if (r.b.x - docTree->size.x <= dr.a.x)
                r.b.x -= docTree->size.x;
            win->locate(r);
        });
    }
    MRUlist.forEach([] (auto *win) {
        win->setState(sfExposed, True);
    });
    deskTop->redraw();
}

void TurboApp::setFocusedEditor(EditorWindow *w)
{
    // w has been focused, so it becomes the first of our MRU list.
    w->MRUhead.insert_after(&MRUlist);
    if (docTree)
        docTree->tree->focusEditor(w);
    // We keep track of the most recent directory for file dialogs.
    if (!w->file.empty())
        mostRecentDir.assign(w->file.parent_path());
}

#define cpTurboAppColor \
    /* 1 = TBackground, 2-7 = TMenuView and TStatusLine */ \
    "\x71\x70\x78\x74\x20\x28\x24" \
    /* 8-15 = TWindow(Blue) */ \
    "\x17\x1F\x1A\x31\x31\x1E\x71\x1F" \
    /* 16-23 = TWindow(Cyan) */ \
    "\x37\x3F\x3A\x13\x13\x3E\x21\x3F" \
    /* 24-31 = TWindow(Gray) */ \
    "\x70\x7F\x7A\x13\x13\x70\x7F\x7E" \
    /* 32-63 = TDialog(Gray) */ \
    "\x70\x7F\x7A\x13\x13\x70\x70\x7F\x7E\x20\x2B\x2F\x78\x2E\x70\x30" \
    "\x3F\x3E\x1F\x2F\x1A\x20\x72\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    /* 64-95 = TDialog(Blue) */ \
    "\x17\x1F\x1A\x71\x71\x1E\x17\x1F\x1E\x20\x2B\x2F\x78\x2E\x10\x30" \
    "\x3F\x3E\x70\x2F\x7A\x20\x12\x31\x31\x30\x2F\x3E\x31\x13\x38\x00" \
    /* 96-127 = TDialog(Cyan) */ \
    "\x37\x3F\x3A\x13\x13\x3E\x30\x3F\x3E\x20\x2B\x2F\x78\x2E\x30\x70" \
    "\x7F\x7E\x1F\x2F\x1A\x20\x32\x31\x71\x70\x2F\x7E\x71\x13\x38\x00" \
    /* 128-135 = Help Colors? */ \
    "\x37\x3F\x3A\x13\x13\x30\x3E\x1E" \
    /* 136-167 = EditorWindow */ \
    "\x07\x0F\x0A\x30\x30\x0F\x08\x0F\x06\x20\x2B\x2F\x78\x2E\x08\x30" \
    "\x3F\x3E\x1F\x2F\x1A\x20\x72\x31\x31\x30\x2F\x3E\x31\x13\x38\x00"

TPalette& TurboApp::getPalette() const {
    static TPalette palette(cpTurboAppColor, sizeof(cpTurboAppColor) - 1);
    return palette;
}
