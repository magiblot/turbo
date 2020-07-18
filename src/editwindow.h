#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"

class DocumentView;

struct EditorWindow : public TWindow, Scintilla::TScintillaWindow {

    EditorWindow(const TRect &bounds);

    DocumentView *docView;
    TDrawableView *leftMargin;
    TScrollBar *vScrollBar;
    bool drawing;

    Scintilla::TScintillaEditor editor;

    void setUpEditor();
    void redrawEditor();
    void setActive(Boolean enable);

    void handleEvent(TEvent &ev) override;

    void scrollBarEvent(TEvent ev);
    bool scrollBarChanged(TScrollBar *bar);
    void setVerticalScrollPos(int delta, int limit, int size) override;

};

#endif
