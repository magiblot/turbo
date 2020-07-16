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
#include <tvision/tv.h>

#include "app.h"
#include "editwindow.h"

using namespace tvedit;
using namespace Scintilla;

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
            *new TStatusItem( 0, kbF10, cmMenu )
            );
}

void TVEditApp::handleEvent(TEvent& event)
{
    TApplication::handleEvent(event);
    if (event.what == evCommand) {
        switch (event.message.command) {
            case cmNew:
                newEditorWindow();
                clearEvent(event);
                break;
            default:
                break;
        }
    }
}

void TVEditApp::newEditorWindow()
{
    EditorWindow *w = new EditorWindow(deskTop->getExtent());
    w = (EditorWindow *) validView(w);
    if (w) {
        deskTop->insert(w);
    }
}
