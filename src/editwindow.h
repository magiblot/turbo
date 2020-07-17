#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"
#include "docview.h"

struct EditorWindow : public TWindow, Scintilla::TScintillaWindow {

    EditorWindow(const TRect &bounds);

    TView *leftSideBar;
    TView *bottomArea;
    DocumentView *docView;

    bool needsRedraw;

    Scintilla::TScintillaEditor editor;

    void setUpEditor();
    void redrawEditor();

    void handleEvent(TEvent &ev) override;
    void changeBounds(const TRect &bounds) override;

    void notify(SCNotification scn) override;

};

#endif
