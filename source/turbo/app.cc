#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TKeys
#define Uses_TMenu
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TWindow
#define Uses_TButton
#define Uses_TFrame
#define Uses_TFileDialog
#define Uses_TIndicator
#define Uses_TStaticText
#define Uses_TParamText
#define Uses_TScreen
#define Uses_MsgBox
#include <tvision/tv.h>

#include "app.h"
#include "apputils.h"
#include "editwindow.h"
#include "widgets.h"
#include "listviews.h"
#include "doctree.h"
#include <turbo/fileeditor.h>
#include <turbo/tpath.h>
#include <turbo/styles.h>
#include <toml.h>
#include <tomlcpp.hpp>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

std::string config_path = "/.config/turbo.toml";

using namespace Scintilla;
using namespace std::literals;

TurboApp* TurboApp::app = 0;
TCommandSet allCmUseLanguages;

int main(int argc, const char *argv[])
{
    config_path.insert(0, getenv("HOME"));

    TurboApp app(argc, argv);
    TurboApp::app = &app;
    app.run();
    app.saveConfig();
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
    // Create the CommandSet for all commands used to select a highlight language. (This lets them all be disabled at once)
    for (int i = 0; (i < turbo::Language::COUNT) && i < (cmUseLanguageMax - cmUseLanguage); i++)
        allCmUseLanguages += (cmUseLanguage + i);

    TCommandSet ts;
    ts += allCmUseLanguages;
    ts += cmSave;
    ts += cmSaveAs;
    ts += cmRename;
    ts += cmOpenRecent;
    ts += cmToggleWrap;
    ts += cmToggleLineNums;
    ts += cmFind;
    ts += cmReplace;
    ts += cmGoToLine;
    ts += cmSearchAgain;
    ts += cmSearchPrev;
    ts += cmToggleIndent;
    ts += cmCloseEditor;
    ts += cmSelUppercase;
    ts += cmSelLowercase;
    ts += cmSelCapitalize;
    ts += cmToggleComment;
    ts += cmUndo;
    ts += cmRedo;
    ts += cmCut;
    ts += cmCopy;
    ts += cmPaste;
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
            r.a.x = r.b.x - min(max(r.b.x - 82, 22), 30);
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

    loadConfig();
}

TMenuBar *TurboApp::initMenuBar(TRect r)
{
    r.b.y = r.a.y+1;

    // Dynamically generate the list of programming languages, each with its own associated cmUseLanguageXXX code
    TMenuItem *langList = NULL;
    for (int i = 0; (i < turbo::Language::COUNT) && i < (cmUseLanguageMax - cmUseLanguage); i++)
    {
        TMenuItem *item = new TMenuItem( turbo::languages[i].name, cmUseLanguage + i, kbNoKey, hcNoContext);
        langList = (langList == NULL) ? item : &(*langList + *item);
    }

    return new TMenuBar( r,
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl-N" ) +
            *new TMenuItem( "~O~pen", cmOpen, kbCtrlO, hcNoContext, "Ctrl-O" ) +
            *new TMenuItem( "Open R~e~cent...", cmOpenRecent, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "~S~ave", cmSave, kbCtrlS, hcNoContext, "Ctrl-S" ) +
            *new TMenuItem( "S~a~ve As...", cmSaveAs, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~R~ename...", cmRename, kbF2, hcNoContext, "F2" ) +
            newLine() +
            *new TMenuItem( "~C~lose", cmCloseEditor, kbCtrlW, hcNoContext, "Ctrl-W" ) +
            *new TMenuItem( "Close All", cmCloseAll, kbNoKey, hcNoContext ) +
            newLine() +
            *new TMenuItem( "S~u~spend", cmDosShell, kbNoKey, hcNoContext ) +
            *new TMenuItem( "E~x~it", cmQuit, kbCtrlQ, hcNoContext, "Ctrl-Q" ) +
        *new TSubMenu( "~E~dit", kbAltE ) +
            *new TMenuItem( "~U~ndo", cmUndo, kbCtrlZ, hcNoContext, "Ctrl-Z" ) +
            *new TMenuItem( "Re~d~o", cmRedo, kbCtrlY, hcNoContext, "Ctrl-Y" ) +
            newLine() +
            *new TMenuItem( "Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl-X" ) +
            *new TMenuItem( "~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl-C" ) +
            *new TMenuItem( "~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl-V" ) +
            newLine() +
            *new TMenuItem( "~F~ind...", cmFind, kbCtrlF, hcNoContext, "Ctrl-F" ) +
            *new TMenuItem( "~R~eplace...",cmReplace, kbCtrlR, hcNoContext, "Ctrl-R" ) +
            *new TMenuItem( "~G~o to Line...",cmGoToLine, kbCtrlG, hcNoContext, "Ctrl-G" ) +
            *new TMenuItem( "Find ~N~ext", cmSearchAgain, kbF3, hcNoContext, "F3" ) +
            *new TMenuItem( "Find ~P~revious", cmSearchPrev, kbShiftF3, hcNoContext, "Shift-F3" ) +
        *new TSubMenu( "Se~l~ection", kbAltL ) +
            *new TMenuItem( "~T~oggle Comment", cmToggleComment, kbCtrlE, hcNoContext, "Ctrl-E" ) +
            newLine() +
            *new TMenuItem( "~U~ppercase", cmSelUppercase, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~L~owercase", cmSelLowercase, kbNoKey, hcNoContext ) +
            *new TMenuItem( "~C~apitalize", cmSelCapitalize, kbNoKey, hcNoContext ) +
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
            *new TMenuItem( "Toggle Document ~T~ree View", cmToggleTree, kbNoKey, hcNoContext ) +
            *new TMenuItem( "Document ~L~anguage", kbNoKey, new TMenu(*langList))
        );
}

TStatusLine *TurboApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( 0, kbAltX, cmQuit ) +
            *new TStatusItem( "~Ctrl-N~ New", kbNoKey, cmNew ) +
            *new TStatusItem( "~Ctrl-O~ Open", kbNoKey, cmOpen ) +
            *new TStatusItem( "~Ctrl-S~ Save", kbNoKey, cmSave ) +
            *new TStatusItem( "~F6~ Next", kbF6, cmEditorNext ) +
            *new TStatusItem( "~F12~ Menu", kbF12, cmMenu ) +
            *new TStatusItem( 0, TKey(kbCtrlZ, kbShift), cmRedo ) +
            *new TStatusItem( 0, kbCtrlX, cmCut ) +
            *new TStatusItem( 0, kbCtrlC, cmCopy ) +
            *new TStatusItem( 0, kbCtrlV, cmPaste ) +
            *new TStatusItem( 0, kbShiftDel, cmCut ) +
            *new TStatusItem( 0, kbCtrlIns, cmCopy ) +
            *new TStatusItem( 0, kbShiftIns, cmPaste ) +
            *new TStatusItem( 0, kbCtrlTab, cmEditorNext ) +
            *new TStatusItem( 0, kbAltTab, cmEditorNext ) +
            *new TStatusItem( 0, kbShiftF6, cmEditorPrev ) +
            *new TStatusItem( 0, TKey(kbCtrlTab, kbShift), cmEditorPrev ) +
            *new TStatusItem( 0, TKey(kbAltTab, kbShift), cmEditorPrev ) +
            *new TStatusItem( 0, TKey('/', kbCtrlShift), cmToggleComment ) +
            *new TStatusItem( 0, TKey('_', kbCtrlShift), cmToggleComment ) +
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

            case cmToggleWrap:
                config.wrapping = !config.wrapping;
                MRUlist.forEach([&] (auto *win) {
                    TurboEditor &editor = win->getEditor();
                    editor.wrapping.setState(config.wrapping, editor.scintilla);
                    editor.redraw();
                });
                break;
            case cmToggleLineNums:
                config.lineNumbers = !config.lineNumbers;
                MRUlist.forEach([&] (auto *win) {
                    win->getEditor().lineNumbers.setState(config.lineNumbers);
                    win->getEditor().redraw();
                });
                break;
            case cmToggleIndent:
                config.autoIndent = !config.autoIndent;
                MRUlist.forEach([&] (auto *win) {
                    win->getEditor().autoIndent.setState(config.autoIndent);
                    win->getEditor().redraw();
                });
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
            current->setText("%s", argv[i]);
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
    if (turbo::readFile(scintilla, abspath, turbo::acceptMissingFilesOnOpen))
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
    return turbo::createScintilla();
}

void TurboApp::addEditor(turbo::TScintilla &scintilla, const char *path)
// Pre: 'path' is an absolute path.
{
    TRect r = newEditorBounds();
    auto &counter = fileCount[TPath::basename(path)];

    auto &editor = *new TurboEditor(scintilla, path);
    editor.lineNumbers.setState(config.lineNumbers);    // Configure with defaults
    editor.autoIndent.setState(config.autoIndent);
    editor.wrapping.setState(config.wrapping, editor.scintilla);

    EditorWindow &w = *new EditorWindow(r, editor, counter, searchSettings, *this);
    if (docTree)
        docTree->tree->addEditor(&w);
    w.listHead.insert_after(&MRUlist);
    deskTop->insert(&w);
    enableCommands(editorCmds);
}

void TurboApp::showEditorList(TEvent *ev)
{
    EditorListModel model {MRUlist};
    TRect r {0, 0, 0, 0};
    r.b.x = min(max(maxWidth(model) + 6, 40), deskTop->size.x - 10);
    r.b.y = min(max(model.size() + 2, 6), deskTop->size.y - 4);
    r.move((deskTop->size.x - r.b.x) / 2,
           (deskTop->size.y - r.b.y) / 4);
    ListWindow *lw = new ListWindow(r, "Buffer List", model, ListViewCreator<EditorListView>());
    if (ev)
        lw->putEvent(*ev);
    if (deskTop->execView(lw) == cmOK)
        if (auto *wnd = (EditorWindow *) lw->getCurrent())
            wnd->focus();

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
        config.mostRecentDir = TPath::dirname(w.filePath());
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
        config.mostRecentDir = TPath::dirname(w.filePath());
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
    return config.mostRecentDir.c_str();
}

#define ERRLEN 100

void TurboApp::loadConfig()
{
    std::string errmsg;

    // Return if config file doesn;t exist
    FILE *f = fopen(config_path.c_str(), "r");
    if (!f) return; // Config file doesn't exist yet. Keep defaults
    fclose(f);

    // Any remaining errors will be parse errors
    auto res = toml::parseFile(config_path);
    if (res.table) {
        bool ok;

        auto editor = res.table->getTable("Editor");
        if (editor) {
            std::tie(ok, config.lineNumbers) = editor->getBool("lineNumbers");
            std::tie(ok, config.autoIndent)  = editor->getBool("autoIndent");
            std::tie(ok, config.wrapping)    = editor->getBool("wrapping");
        }

        auto state = res.table->getTable("State");
        if (state) {
            std::tie(ok, config.mostRecentDir) = state->getBool("mostRecentDir");
        }

        return;
    }

    messageBox( mfError | mfOKButton, "Error loading config:\n%s", res.errmsg.c_str());
}

void TurboApp::saveConfig()
{
    // https://github.com/cktan/tomlc99/issues/91

    FILE *out = fopen(config_path.c_str(), "w");

    fprintf(out, "[Editor]\n");
    fprintf(out, "lineNumbers = %s\n", config.lineNumbers ? "true" : "false");
    fprintf(out, "autoIndent = %s\n", config.autoIndent ? "true" : "false");
    fprintf(out, "wrapping = %s\n", config.wrapping ? "true" : "false");
    fprintf(out, "\n");
    fprintf(out, "[State]\n");
    fprintf(out, "mostRecentDir = \"%s\"\n", config.mostRecentDir.c_str());

    fclose(out);
}