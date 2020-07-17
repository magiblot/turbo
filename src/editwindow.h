#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"

class DocumentView;

struct EditorWindow : public TWindow {

    EditorWindow(const TRect &bounds);

    DocumentView *docView;
    TDrawableView *leftMargin;
    TScrollBar *hScrollBar, *vScrollBar;

    Scintilla::TScintillaEditor editor;

    void setUpEditor();
    void redrawEditor();
    void setActive(Boolean enable);

};

#endif
