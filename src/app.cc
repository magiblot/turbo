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

#include "app.h"
#include "editwindow.h"
#include "util.h"

using namespace tvedit;
using namespace Scintilla;
using namespace std::literals;

TVEditApp* tvedit::app = 0;

TVEditApp::TVEditApp() :
    TProgInit( &TVEditApp::initStatusLine,
               &TVEditApp::initMenuBar,
               &TApplication::initDeskTop
             )
{
}

TMenuBar *TVEditApp::initMenuBar(TRect r)
{
    r.b.y = r.a.y+1;
    return new TMenuBar( r,
        *new TSubMenu( "~F~ile", kbAltF, hcNoContext ) +
            *new TMenuItem( "~N~ew", cmNew, kbCtrlN, hcNoContext, "Ctrl+N" ) +
            *new TMenuItem( "~O~pen", cmOpen, kbCtrlO, hcNoContext, "Ctrl+O" ) +
            newLine() +
            *new TMenuItem( "E~x~it", cmQuit, kbNoKey, hcNoContext, "Alt+X" )
            );

}

TStatusLine *TVEditApp::initStatusLine( TRect r )
{
    r.a.y = r.b.y-1;
    return new TStatusLine( r,
        *new TStatusDef( 0, 0xFFFF ) +
            *new TStatusItem( "~Alt+X~ Exit", kbAltX, cmQuit ) +
            *new TStatusItem( 0, kbF10, cmMenu )
            );
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
    if (w) {
        setEditorTitle(w);
        deskTop->insert(w);
    }
    return w;
}

void TVEditApp::setEditorTitle(EditorWindow *w)
{
    char buf[12];
    uint number;
    const char *file = w->getTitle(0U);
    if (file) {
        w->title.assign(file);
        number = ++editorTitles[file];
    } else {
        w->title.assign("Untitled"s);
        number = ++editorTitles[""];
    }
    if (number > 1) {
        std::string_view n = {buf, fast_utoa(number, buf)};
        w->title.append(" ("sv);
        w->title.append(n);
        w->title.push_back(')');
    }
}
