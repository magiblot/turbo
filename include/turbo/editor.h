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
    // This gets invoked when certain situations occur in 'Editor'. See the
    // 'notification codes' in 'Editor' and its subclasses.
    virtual void handleNotification(ushort code, Editor &) noexcept = 0;
};

struct Editor : TScintillaParent
{
    // 'Editor' is a bridge between 'TScintilla' and Turbo Vision.

    // Notification Codes for EditorParent::handleNotification.
    enum : ushort {
        ncPainted = 1, // Editor has been drawn into its associated views.
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
    // Receives notifications in certain situations.
    EditorParent *parent {nullptr};

    // These views reflect the editor's state and feed events to it.
    EditorView *view {nullptr};
    LeftMarginView *leftMargin {nullptr};
    TScrollBar *hScrollBar {nullptr};
    TScrollBar *vScrollBar {nullptr};

    // Draw state.
    TDrawSurface surface;
    InvalidationRectangle invalidatedArea;
    bool drawLock {false}; // To avoid recursive draws.
    bool resizeLock {false}; // When true, text stops flowing on resize.

    // Things related to 'TScintilla's state.
    LineNumbersWidth lineNumbers {minLineNumbersWidth};
    WrapState wrapping;
    AutoIndent autoIndent;
    ThemingState theming;

    // Takes ownership over 'aScintilla'.
    Editor(TScintilla &aScintilla) noexcept;
    virtual ~Editor();

    // Causes the editor to be drawn in the provided views. All arguments are
    // nullable.
    void associate( EditorParent *aParent,
                    EditorView *aView, LeftMarginView *aLeftMargin,
                    TScrollBar *aHScrollBar, TScrollBar *aVScrollBar ) noexcept;
    void disassociate() noexcept;

    void scrollBarEvent(TEvent &ev);
    void scrollTo(TPoint delta) noexcept;
    bool handleScrollBarChanged(TScrollBar *);

    // Paints the whole editor.
    void redraw() noexcept;
    // Paints the editor in the area indicated by 'invalidatedArea'.
    void partialRedraw() noexcept;
    bool redraw(const TRect &area) noexcept;
    void drawViews() noexcept;

    // Sets the bounds of 'view' and 'leftMargin' according to 'lineNumbers'.
    void updateMarginWidth() noexcept;

    // Implementation of 'TScintillaParent'.
    TPoint getEditorSize() noexcept override;
    void invalidate(TRect area) noexcept override;
    void handleNotification(const SCNotification &scn) override;
    void setHorizontalScrollPos(int delta, int limit) noexcept override;
    void setVerticalScrollPos(int delta, int limit) noexcept override;

    // * 'confirmWrap' shall return whether line wrapping should be activated
    //   even if the document is quite large (>= 512 KiB).
    // Returns whether line wrapping has been enabled.
    inline bool toggleLineWrapping(TFuncView<bool(int width)> confirmWrap = WrapState::defConfirmWrap) noexcept;
    inline void toggleLineNumbers() noexcept;
    inline void toggleAutoIndent() noexcept;
    bool inSavePoint();
    inline sptr_t callScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam);

};

inline bool Editor::toggleLineWrapping(TFuncView<bool(int)> confirmWrap) noexcept
{
    return wrapping.toggle(scintilla, confirmWrap);
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
    // 'EditorView' is used to display an 'Editor's contents and feed input
    // events (keboard, mouse) to it.

    // During an invocation to 'editor->redraw()', the 'TSurfaceView::surface'
    // member is temporally set to 'editor->surface'. If 'this->draw()' is invoked
    // and 'TSurfaceView::surface' is null, 'editor->redraw()' also gets called.
    // To avoid issues during window resize, make sure the 'EditorView' is
    // inserted before the other views associated to 'editor' (left margin,
    // scrollbars...).
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
    // 'LeftMarginView' is used to display the left margin (line numbers)
    // of an 'Editor' separately from 'EditorView'. This can be useful
    // if you want to draw a frame around it.
public:

    int distanceFromView;

    LeftMarginView(int aDistance) noexcept;
};

} // namespace turbo

#endif // TURBO_EDITOR_H
