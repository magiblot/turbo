#ifndef TVEDIT_EDITWINDOW_H
#define TVEDIT_EDITWINDOW_H

#define Uses_TWindow
#include <tvision/tv.h>

#include "tscintilla.h"
#include "drawviews.h"
#include "util.h"

#include <string_view>
#include <string>
#include <filesystem>

class DocumentView;

struct EditorWindow : public TWindow, Scintilla::TScintillaWindow {

    EditorWindow(const TRect &bounds, std::string_view aFile);
    ~EditorWindow();

    // Subviews

    DocumentView *docView;
    TDrawSubView *leftMargin;
    TScrollBar *hScrollBar, *vScrollBar;
    bool drawing;

    // Scintilla

    Scintilla::TScintillaEditor editor;
    TDrawableView editorView;

    TRect editorBounds() const;
    void setUpEditor();
    void redrawEditor();

    void handleEvent(TEvent &ev) override;
    void changeBounds(const TRect &bounds) override;
    void setState(ushort aState, Boolean enable) override;
    Boolean valid(ushort command) override;
    const char* getTitle(short) override;
    void sizeLimits(TPoint &min, TPoint &max);

    void lockSubViews();
    void unlockSubViews();
    void scrollBarEvent(TEvent ev);
    bool scrollBarChanged(TScrollBar *bar);
    void scrollTo(TPoint delta);
    void setHorizontalScrollPos(int delta, int limit) override;
    void setVerticalScrollPos(int delta, int limit) override;

    // TVEditApp integration

    list_head<EditorWindow> MRUhead;

    // File management

    std::filesystem::path file;
    std::string error;
    std::string title; // Later set by TVEditApp

    void tryLoadFile();
    void loadFile();

    static constexpr TPoint minEditWinSize {24, 6};

};

#endif
