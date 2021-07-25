#ifndef TURBO_TURBO_H
#define TURBO_TURBO_H

#define Uses_TRect
#define Uses_TSurfaceView
#include <tvision/tv.h>

#include <turbo/funcview.h>
#include <turbo/scintilla.h>
#include <turbo/editstates.h>
#include <turbo/styles.h>

#include <string>
#include <utility>

class TScrollBar;

namespace turbo {

class Clipboard;
class EditorView;
class LeftMarginView;
struct EditorState;

struct EditorParent
{
    virtual void handleNotification(ushort code, EditorState &) noexcept = 0;
};

struct EditorState : ScintillaParent
{
    enum : ushort { // Notification Codes
        ncPainted = 1,
    };

    struct InvalidationRectangle : TRect
    {
        using TRect::TRect;
        using TRect::operator=;
        InvalidationRectangle() : TRect(-1, -1, -1, -1) {}
        void clear() { a.x = -1; }
        bool empty() const { return a.x < 0; }
    };

    enum { minLineNumbersWidth = 5 };

    Scintilla &scintilla;
    ThemingState theming;
    EditorParent *parent {nullptr};
    EditorView *view {nullptr};
    LeftMarginView *leftMargin {nullptr};
    TScrollBar *hScrollBar {nullptr};
    TScrollBar *vScrollBar {nullptr};
    TDrawSurface surface;
    InvalidationRectangle invalidatedArea;
    bool drawLock {false};
    bool resizeLock {false};
    LineNumbersWidth lineNumbers {minLineNumbersWidth};
    WrapState wrapping;
    AutoIndent autoIndent;

    EditorState(Scintilla &aScintilla) noexcept;
    virtual ~EditorState();

    void associate( EditorParent *aParent,
                    EditorView *aView, LeftMarginView *aLeftMargin,
                    TScrollBar *aHScrollBar, TScrollBar *aVScrollBar ) noexcept;
    void disassociate() noexcept;

    void scrollBarEvent(TEvent &ev);
    void scrollTo(TPoint delta) noexcept;
    void redraw() noexcept;
    void partialRedraw() noexcept;
    bool redraw(const TRect &area) noexcept;
    void drawViews() noexcept;
    void updateMarginWidth() noexcept;
    bool handleScrollBarChanged(TScrollBar *);

    TPoint getEditorSize() noexcept override;
    void invalidate(TRect area) noexcept override;
    void handleNotification(const SCNotification &scn) override;
    void setHorizontalScrollPos(int delta, int limit) noexcept override;
    void setVerticalScrollPos(int delta, int limit) noexcept override;

    bool toggleLineWrapping(TFuncView<bool(int)> wrapIfBig = defWrapIfBig) noexcept
    // Post: returns true if line wrapping is enabled.
    {
        return wrapping.toggle(scintilla, wrapIfBig);
    }

    void toggleLineNumbers() noexcept
    {
        lineNumbers.enabled ^= true;
    }

    void toggleAutoIndent() noexcept
    {
        autoIndent.enabled ^= true;
    }

    bool inSavePoint();

};

class EditorView : public TSurfaceView
{
public:

    EditorState *editorState {nullptr};

    EditorView(const TRect &bounds) noexcept;

    void handleEvent(TEvent &ev) override;
    void draw() override;

private:

    void consumeInputText(TEvent &ev);

};

class LeftMarginView : public TSurfaceView
{
public:

    int distanceFromView;

    LeftMarginView(int aDistance) noexcept;
};

struct FileEditorState;

struct FileDialogs
{
    virtual ushort confirmSaveUntitled(FileEditorState &) noexcept = 0;
    virtual ushort confirmSaveModified(FileEditorState &) noexcept = 0;
    virtual ushort confirmOverwrite(const char *path) noexcept = 0;
    virtual void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept = 0;
    virtual bool renameError(const char *dst, const char *src, const char *cause) noexcept = 0;
    virtual bool fileTooBigError(const char *path, size_t size) noexcept = 0;
    virtual bool readError(const char *path, const char *cause) noexcept = 0;
    virtual bool writeError(const char *path, const char *cause) noexcept = 0;
    virtual bool openForReadError(const char *path, const char *cause) noexcept = 0;
    virtual bool openForWriteError(const char *path, const char *cause) noexcept = 0;
    virtual void getOpenPath(TFuncView<bool (const char *)> accept) noexcept = 0;
    virtual void getSaveAsPath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept = 0;
    virtual void getRenamePath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept = 0;
};

struct DefaultFileDialogs : FileDialogs
{
    ushort confirmSaveUntitled(FileEditorState &) noexcept override;
    ushort confirmSaveModified(FileEditorState &) noexcept override;
    ushort confirmOverwrite(const char *path) noexcept override;
    void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept override;
    bool renameError(const char *dst, const char *src, const char *cause) noexcept override;
    bool fileTooBigError(const char *path, size_t size) noexcept override;
    bool readError(const char *path, const char *cause) noexcept override;
    bool writeError(const char *path, const char *cause) noexcept override;
    bool openForReadError(const char *path, const char *cause) noexcept override;
    bool openForWriteError(const char *path, const char *cause) noexcept override;
    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;
};

extern DefaultFileDialogs defFileDialogs;

struct SilentFileDialogs : FileDialogs
{
    ushort confirmSaveUntitled(FileEditorState &) noexcept override;
    ushort confirmSaveModified(FileEditorState &) noexcept override;
    ushort confirmOverwrite(const char *path) noexcept override;
    void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept override;
    bool renameError(const char *dst, const char *src, const char *cause) noexcept override;
    bool fileTooBigError(const char *path, size_t size) noexcept override;
    bool readError(const char *path, const char *cause) noexcept override;
    bool writeError(const char *path, const char *cause) noexcept override;
    bool openForReadError(const char *path, const char *cause) noexcept override;
    bool openForWriteError(const char *path, const char *cause) noexcept override;
    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;
};

extern SilentFileDialogs silFileDialogs;

bool readFile(Scintilla &scintilla, const char *path, FileDialogs & = defFileDialogs) noexcept;
void openFile( TFuncView<Scintilla&()> createScintilla,
               TFuncView<void(Scintilla &, const char *)> accept,
               FileDialogs & = defFileDialogs ) noexcept;
bool writeFile(const char *path, Scintilla &scintilla, FileDialogs & = defFileDialogs) noexcept;
bool renameFile(const char *dst, const char *src, Scintilla &scintilla, FileDialogs & = defFileDialogs) noexcept;

struct FileEditorState : EditorState
{
    enum : ushort { // Notification Codes
        ncSaved = 100,
    };

    std::string filePath;

    FileEditorState(Scintilla &aScintilla, std::string aFilePath) noexcept;

    void detectLanguage() noexcept;
    bool save(FileDialogs & = defFileDialogs) noexcept;
    bool saveAs(FileDialogs & = defFileDialogs) noexcept;
    bool rename(FileDialogs & = defFileDialogs) noexcept;
    bool close(FileDialogs & = defFileDialogs) noexcept;

    void beforeSave() noexcept;
    virtual void afterSave() noexcept;
    void notifyAfterSave() noexcept;

};

inline FileEditorState::FileEditorState(Scintilla &aScintilla, std::string aFilePath) noexcept :
    EditorState(aScintilla),
    filePath(std::move(aFilePath))
{
    detectLanguage();
}

inline void FileEditorState::detectLanguage() noexcept
{
    theming.detectLanguage(filePath.c_str(), scintilla);
}

} // namespace turbo

#endif // TURBO_TURBO_H
