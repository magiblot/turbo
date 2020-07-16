#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"
#include "docview.h"

struct EditorWindow : public TWindow {

    EditorWindow(const TRect &bounds);

    TView *leftSideBar;
    TView *bottomArea;
    DocumentView *docView;

    Scintilla::TScintillaEditor editor;

};

#endif
