#ifndef TURBO_TURBO_H
#define TURBO_TURBO_H

#define Uses_TRect
#define Uses_TSurfaceView
#include <tvision/tv.h>

#include <tscintilla.h>
#include <editstates.h>
#include <styles.h>

#include <string>
#include <utility>

class TScrollBar;

namespace turbo {
namespace constants {
enum : ushort {
    // Line Wrapping options
    lwConfirm = 0x0001, // Ask for confirmation when document too big.
};
} // namespace constants

using Editor = Scintilla::TScintillaEditor;
class EditorView;
class LeftMarginView;

Editor &createEditor();

struct EditorState : Scintilla::TScintillaParent
{

    struct InvalidationRectangle : TRect
    {
        using TRect::TRect;
        using TRect::operator=;
        InvalidationRectangle() : TRect(-1, -1, -1, -1) {}
        void clear() { a.x = -1; }
        bool empty() const { return a.x < 0; }
    };

    enum { minLineNumbersWidth = 5 };

    Editor &editor;
    ThemingState theming;
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

    EditorState(Editor &aEditor);
    virtual ~EditorState();

    void associate( EditorView *aView, LeftMarginView *aLeftMargin,
                    TScrollBar *aHScrollBar, TScrollBar *aVScrollBar );
    void disassociate();

    void scrollBarEvent(TEvent &ev);
    void scrollTo(TPoint delta);
    void redraw();
    void partialRedraw();
    bool redraw(const TRect &area);
    virtual void drawViews();
    void updateMarginWidth();
    bool handleScrollBarChanged(TScrollBar *);

    TPoint getEditorSize() override;
    void invalidate(TRect area) override;
    void setHorizontalScrollPos(int delta, int limit) override;
    void setVerticalScrollPos(int delta, int limit) override;

    bool toggleLineWrapping(ushort options)
    // Post: returns true if line wrapping is enabled.
    {
        return wrapping.toggle(editor, options & constants::lwConfirm);
    }

    void toggleLineNumbers()
    {
        lineNumbers.enabled ^= true;
    }

    bool inSavePoint();

};

class EditorView : public TSurfaceView
{
public:

    EditorState *state {nullptr};

    EditorView(const TRect &bounds);

    void handleEvent(TEvent &ev) override;
    void draw() override;

private:

    void consumeInputText(TEvent &ev);

};

class LeftMarginView : public TSurfaceView
{
public:

    int distanceFromView;

    LeftMarginView(int aDistance);
};

namespace constants {
enum : ushort {
    // Open File options
    ofShowError = 0x0001, // Show a dialog on error.
};
} // namespace turbo::constants

Editor *openFile(const char *filePath, ushort options);

struct OpenFileWithDialogResult
{
    Editor *editor;
    std::string filePath;
};

OpenFileWithDialogResult openFileWithDialog(const char *dir = nullptr);

struct FileEditorState : EditorState
{

    std::string filePath;

    FileEditorState(Editor &aEditor, std::string aFilePath);

};

inline FileEditorState::FileEditorState(Editor &aEditor, std::string aFilePath) :
    EditorState(aEditor),
    filePath(std::move(aFilePath))
{
    if (theming.detectLanguage(filePath.c_str(), editor))
        lineNumbers.enabled = true;
}

} // namespace turbo

#endif // TURBO_TURBO_H
