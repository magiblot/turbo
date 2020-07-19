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
#include <tvision/tv.h>

#include <fmt/core.h>

#include "app.h"
#include "editwindow.h"
#include "util.h"
#include "widgets.h"

using namespace Scintilla;
using namespace std::literals;

TVEditApp* TVEditApp::app = 0;

TVEditApp::TVEditApp() :
    TProgInit( &TVEditApp::initStatusLine,
               &TVEditApp::initMenuBar,
               &TApplication::initDeskTop
             )
{
    // Create the clock view.
    TRect r = getExtent();
    r.a.x = r.b.x - 9;
    r.b.y = r.a.y + 1;
    clock = new TClockView(r);
    clock->growMode = gfGrowLoX | gfGrowHiX;
    insert(clock);
}

TMenuBar *TVEditApp::initMenuBar(TRect r)
{
    r.b.y = r.a.y+1;
    return new TMenuBar( r,
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl-N" ) +
            *new TMenuItem( "~O~pen", cmOpen, kbCtrlO, hcNoContext, "Ctrl-O" ) +
            newLine() +
            *new TMenuItem( "~C~lose", cmClose, kbCtrlW, hcNoContext, "Ctrl-W" ) +
            newLine() +
            *new TMenuItem( "E~x~it", cmQuit, kbNoKey, hcNoContext, "Alt-X" )
            );

}

TStatusLine *TVEditApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Alt-X~ Exit", kbAltX, cmQuit ) +
            *new TStatusItem( "~Ctrl-N~ New", kbCtrlN, cmNew ) +
            *new TStatusItem( "~Ctrl-O~ Open", kbCtrlO, cmOpen ) +
            *new TStatusItem( "~Ctrl-W~ Close", kbCtrlW, cmClose ) +
            *new TStatusItem( "~F10~ Menu" , kbF10, cmMenu )
            );
}

void TVEditApp::idle()
{
    clock->update();
}

void TVEditApp::handleEvent(TEvent& event)
{
    TApplication::handleEvent(event);
    bool handled = false;
    if (event.what == evCommand) {
        handled = true;
        switch (event.message.command) {
            case cmNew: fileNew(); break;
            case cmOpen: fileOpen(); break;
            default:
                handled = false;
                break;
        }
    }
    if (handled)
        clearEvent(event);
}

void TVEditApp::fileNew()
{
    openEditor({});
}

void TVEditApp::fileOpen()
{
    // Unfortunately, TFileDialog relies on the current directory.
    chdir(getMostRecentDir().c_str());
    auto *dialog = new TFileDialog( "*.*",
                                    "Open file",
                                    "~N~ame",
                                    fdOpenButton,
                                    0 );
    execDialog(dialog, nullptr, [this] (ushort, TView *dialog) {
        // MAXPATH as assumed by TFileDialog.
        char fileName[MAXPATH];
        dialog->getData(fileName);
        return openEditor(fileName);
    });
}

bool TVEditApp::openEditor(std::string_view fileName)
{
    EditorWindow *w = new EditorWindow(deskTop->getExtent(), fileName);
    w = (EditorWindow *) validView(w);
    if (w)
        addEditor(w);
    return w;
}

void TVEditApp::setEditorTitle(EditorWindow *w)
{
    uint number;
    std::string_view file = w->file.native();
    if (!file.empty()) {
        w->title.assign(file);
        number = ++getFileCounter(file);
    } else {
        w->title.assign("Untitled"s);
        number = ++fileCount[{}];
    }
    if (number > 1)
        w->title.append(fmt::format(" ({})", number));
}

active_counter& TVEditApp::getFileCounter(std::string_view file)
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

void TVEditApp::addEditor(EditorWindow *w)
{
    setEditorTitle(w);
    w->MRUhead.insert_after(&MRUlist);
    deskTop->insert(w);
}

void TVEditApp::removeEditor(EditorWindow *w)
{
    --getFileCounter(w->file.native());
}

void TVEditApp::tellFocusedEditor(EditorWindow *w)
{
    // w has been focused, so it becomes the first of our MRU list.
    w->MRUhead.remove();
    w->MRUhead.insert_after(&MRUlist);
}

std::filesystem::path TVEditApp::getMostRecentDir()
{
    auto *head = MRUlist.next;
    // Iterate over the list to skip Untitled editors.
    while (head->self) {
        if (!head->self->file.empty())
            return head->self->file.parent_path();
        head = head->next;
    }
    return ".";
}
