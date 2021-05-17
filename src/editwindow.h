#ifndef TURBO_EDITWINDOW_H
#define TURBO_EDITWINDOW_H

#define Uses_TWindow
#define Uses_TPalette
#define Uses_TSurfaceView
#include <tvision/tv.h>

#include "tscintilla.h"
#include "util.h"
#include "editstates.h"
#include "editsurface.h"
#include "styles.h"

#include <string_view>
#include <string>

struct DocumentView;
struct SearchBox;
class ScopedGuard;

struct BaseEditorWindow : public TWindow, Scintilla::TScintillaWindow {

    static TFrame* initFrame(TRect bounds);

    BaseEditorWindow( const TRect &bounds );
    virtual ~BaseEditorWindow();

    // Subviews

    DocumentView *docView;
    TSurfaceView *leftMargin;
    SearchBox *search;
    TScrollBar *hScrollBar, *vScrollBar;
    TCommandSet enabledCmds, disabledCmds;
    bool drawing;
    bool resizeLock;
    TPoint lastSize;
    LineNumbersWidth lineNumbers;
    ::WrapMode wrap;
    Indent indent;
    DocumentProperties props;
    ThemingState theming;

    // Scintilla

    Scintilla::TScintillaEditor editor;
    EditorSurface editorView;

    TPoint editorSize() const;
    TPoint cursorPos();
    void setUpEditorPreLoad();
    void loadText(std::string_view text);
    void setUpEditorPostLoad();
    void redrawEditor();
    void updateMarginWidth();
    void updateIndicatorValue();

    void handleEvent(TEvent &ev) override;
    void changeBounds(const TRect &bounds) override;
    void dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize) override;
    void setState(ushort aState, Boolean enable) override;

    // Minimum window size

    void sizeLimits(TPoint &min, TPoint &max) override;
    static constexpr TPoint minEditWinSize {24, 6};

    void updateCommands();
    void lockSubViews();
    void unlockSubViews();
    void scrollBarEvent(TEvent ev);
    bool scrollBarChanged(TScrollBar *bar);
    void scrollTo(TPoint delta);
    void notify(SCNotification scn) override;
    void setHorizontalScrollPos(int delta, int limit) override;
    void setVerticalScrollPos(int delta, int limit) override;

    ScopedGuard lockDrawing();
};

struct EditorWindow : public BaseEditorWindow {
    EditorWindow( const TRect &bounds );
    virtual ~EditorWindow();
    void setState(ushort aState, Boolean enable) override;
    void setUpEditor(std::string_view aFile, bool openCanFail);
    void handleEvent(TEvent &ev) override;
    Boolean valid(ushort command) override;
    const char* getTitle(short) override;
    void notify(SCNotification scn) override;
    TPalette& getPalette() const override;

    // TurboApp integration

    list_head<EditorWindow> MRUhead;

    // File management

    // The absolute path of the currently open file, or an empty path if
    // no file is open.

    std::string file;
    virtual void setFile(std::string); // Use this setter to update the string.

    // If there was an error while loading the file, the view is invalid.
    // It shall return False when invoking valid(cmValid).

    bool fatalError;

    // The title of the window. It depends on TurboApp, which keeps track of
    // things such as the number of editors open on the same file.
    // 'title' and 'name' are the same, except that 'title' is also modified
    // when a save point is reached or left.

    std::string title, name;
    bool inSavePoint;

    // These two functions modify the 'title' and 'inSavePoint' variables.
    // They may be invoked due to a Scintilla notification or because the file
    // was just saved and the title must be updated.

    void setSavePointLeft();
    void setSavePointReached();

    // The following is used to notify Scintilla about a save point. But it
    // also sets inSavePoint as if the title had been properly updated.

    void setSavePoint();

    // tryLoadFile is invoked when creating the Window. It decides whether
    // a file should be opened, according to 'file'.

    void tryLoadFile(std::string_view aFile, bool canFail);
    bool loadFile(const char *src, bool canFail);

    // trySaveFile gets invoked on cmSave. It decides whether to save the
    // file or to show a dialog. It also takes care of updating the window
    // title if necessary.

    bool trySaveFile();
    void processBeforeSave();
    void processAfterSave();
    bool saveFile(const char *dst, bool silent=false);
    bool renameFile(const char *src, const char *dst);
    static bool canOverwrite(const char *);

    // saveAsDialog keeps popping out a dialog until the user decides
    // not to save or we succeed in writing to file. It also updates
    // the window title.

    bool saveAsDialog();
    void renameDialog();

    bool tryClose();
    void close() override;

    // Pops out a msgBox with an error message.

    static void showError(std::string_view s);
    static void showWarning(std::string_view s);
};

class ScopedGuard
{
    bool &dest;
    const bool value;

public:

    ScopedGuard(bool &dest, bool guardValue) :
        dest(dest),
        value(dest)
    {
        dest = guardValue;
    }

    ~ScopedGuard()
    {
        dest = value;
    }
};

inline ScopedGuard BaseEditorWindow::lockDrawing()
{
    return ScopedGuard(drawing, true);
}

#endif
