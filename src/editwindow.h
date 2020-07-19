#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"
#include "drawviews.h"
#include <string_view>
#include <string>

class FileEditor;

struct EditorWindow : public TWindow, Scintilla::TScintillaWindow {

    EditorWindow(const TRect &bounds, std::string_view aFileName);

    FileEditor *docView;
    TDrawSubView *leftMargin;
    TScrollBar *vScrollBar;
    bool drawing;
    std::string error;
    std::string title;

    Scintilla::TScintillaEditor editor;
    TDrawableView editorView;

    TRect editorBounds() const;
    void setUpEditor();
    void redrawEditor();
    void setActive(Boolean enable);

    void handleEvent(TEvent &ev) override;
    void changeBounds(const TRect &bounds) override;
    Boolean valid(ushort command) override;
    const char* getTitle(short) override;

    void lockSubViews();
    void unlockSubViews();
    void scrollBarEvent(TEvent ev);
    bool scrollBarChanged(TScrollBar *bar);
    void setVerticalScrollPos(int delta, int limit, int size) override;

};

#endif
