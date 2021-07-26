#ifndef TURBO_EDITOR_H
#define TURBO_EDITOR_H

#define Uses_TRect
#define Uses_TSurfaceView
#include <tvision/tv.h>

#include <turbo/funcview.h>
#include <turbo/scintilla.h>
#include <turbo/editstates.h>
#include <turbo/styles.h>

class TScrollBar;

namespace turbo {

class Clipboard;
class EditorView;
class LeftMarginView;
struct Editor;

struct EditorParent
{
    virtual void handleNotification(ushort code, Editor &) noexcept = 0;
};

struct Editor : TScintillaParent
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

    TScintilla &scintilla;
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

    Editor(TScintilla &aScintilla) noexcept;
    virtual ~Editor();

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

    inline bool toggleLineWrapping(TFuncView<bool(int)> wrapIfBig = defWrapIfBig) noexcept;
    inline void toggleLineNumbers() noexcept;
    inline void toggleAutoIndent() noexcept;
    bool inSavePoint();
    inline sptr_t callScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam);

};

inline bool Editor::toggleLineWrapping(TFuncView<bool(int)> wrapIfBig) noexcept
// Post: returns whether line wrapping is enabled.
{
    return wrapping.toggle(scintilla, wrapIfBig);
}

inline void Editor::toggleLineNumbers() noexcept
{
    lineNumbers.enabled ^= true;
}

inline void Editor::toggleAutoIndent() noexcept
{
    autoIndent.enabled ^= true;
}

inline sptr_t Editor::callScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return turbo::call(scintilla, iMessage, wParam, lParam);
}

class EditorView : public TSurfaceView
{
public:

    Editor *editor {nullptr};

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

} // namespace turbo

#endif // TURBO_EDITOR_H
