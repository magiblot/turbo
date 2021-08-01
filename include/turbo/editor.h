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
class Editor;

struct EditorParent
{
    // This allows the parent of 'Editor' to handle TScintilla notifications.
    // See 'https://www.scintilla.org/ScintillaDoc.html#Notifications'.
    virtual void handleNotification(const SCNotification &scn, Editor &) = 0;
};

class Editor : protected TScintillaParent
{
    // 'Editor' is a bridge between 'TScintilla' and Turbo Vision.

    struct InvalidationRectangle : TRect
    {
        using TRect::TRect;
        using TRect::operator=;
        InvalidationRectangle() : TRect(-1, -1, -1, -1) {}
        void clear() { a.x = -1; }
        bool empty() const { return a.x < 0; }
    };

    enum { minLineNumbersWidth = 5 };

    // Draw state.
    TDrawSurface surface;
    InvalidationRectangle invalidatedArea;
    bool drawLock {false}; // To avoid recursive draws.
    bool reflowLock {false}; // When true, text stops flowing on resize.

    void drawViews() noexcept;
    void updateMarginWidth() noexcept;

protected:

    // Implementation of 'TScintillaParent'.
    TPoint getEditorSize() noexcept override;
    void invalidate(TRect area) noexcept override;
    void handleNotification(const SCNotification &scn) override;
    void setHorizontalScrollPos(int delta, int limit) noexcept override;
    void setVerticalScrollPos(int delta, int limit) noexcept override;

public:

    TScintilla &scintilla;

    // Things related to 'TScintilla's state.
    LineNumbersWidth lineNumbers {minLineNumbersWidth};
    WrapState wrapping;
    AutoIndent autoIndent;
    ThemingState theming;

    // Interaction with views. Set them with 'associate'/'disassociate'.
    EditorParent *parent {nullptr}; // Receives notifications in certain situations.
    EditorView *view {nullptr}; // This and the ones below reflect the editor's state and feed events to it.
    LeftMarginView *leftMargin {nullptr};
    TScrollBar *hScrollBar {nullptr};
    TScrollBar *vScrollBar {nullptr};

    // Takes ownership over 'aScintilla'.
    Editor(TScintilla &aScintilla) noexcept;
    virtual ~Editor();

    // Causes the editor to be drawn in the provided views. All parameters are
    // nullable and non-owning and their lifetime must exceed that of 'this'.
    void associate( EditorParent *aParent,
                    EditorView *aView, LeftMarginView *aLeftMargin,
                    TScrollBar *aHScrollBar, TScrollBar *aVScrollBar ) noexcept;
    void disassociate() noexcept;

    void scrollBarEvent(TEvent &ev);
    void scrollTo(TPoint delta) noexcept;
    bool handleScrollBarChanged(TScrollBar *);

    // Paints the whole editor.
    void redraw() noexcept;
    // Paints only the area invalidated by changes in 'scintilla'.
    void partialRedraw() noexcept;
    bool redraw(const TRect &area) noexcept;

    bool inSavePoint();
    template <class Func>
    inline void lockReflow(Func &&func);
    inline sptr_t callScintilla(unsigned int iMessage, uptr_t wParam, sptr_t lParam);
};

template <class Func>
inline void Editor::lockReflow(Func &&func)
{
    bool lastLock = reflowLock;
    reflowLock = true;
    func();
    reflowLock = lastLock;
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

    Editor *editor {nullptr}; // Non-owning.

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

inline void drawWithSurface(TSurfaceView &view, TDrawSurface *surface)
{
    auto *lastSurface = view.surface;
    view.surface = surface;
    view.drawView();
    view.surface = lastSurface;
}

} // namespace turbo

#endif // TURBO_EDITOR_H
