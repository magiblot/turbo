#define Uses_TScrollBar
#include <tvision/tv.h>

#include "docview.h"
#include "editwindow.h"

using namespace Scintilla;

DocumentView::DocumentView( const TRect &bounds,
                            const TDrawableView &view,
                            Scintilla::TScintillaEditor &aEditor,
                            EditorWindow &aWindow ) :
    TDrawSubView(bounds, view),
    editor(aEditor),
    window(aWindow)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable;
    eventMask |= evMouseUp | evMouseMove | evMouseAuto;
    showCursor();
}

void DocumentView::handleEvent(TEvent &ev)
{
    TView::handleEvent(ev);
    bool handled = false;
    switch (ev.what) {
        case evKeyDown:
            if (ev.keyDown.keyCode == kbIns)
                setState(sfCursorIns, Boolean(!getState(sfCursorIns)));
            editor.KeyDownWithModifiers(ev.keyDown, nullptr);
            handled = true;
            break;
        case evMouseDown:
            do {
                TPoint where = makeLocal(ev.mouse.where) + delta;
                if (ev.what == evMouseWheel) {
                    // Mouse wheel while holding button down.
                    window.scrollBarEvent(ev);
                    ev.mouse.where = where;
                    ev.what = evMouseMove;
                    // For some reason, the caret is not always updated
                    // unless this is invoked twice.
                    editor.MouseEvent(ev);
                    editor.MouseEvent(ev);
                } else {
                    ev.mouse.where = where;
                    if (!editor.MouseEvent(ev))
                        break;
                }
                window.redrawEditor();
                handled = true;
            } while (mouseEvent(ev, evMouseDown | evMouseMove | evMouseAuto | evMouseWheel));
            break;
    }
    if (handled) {
        window.redrawEditor();
        clearEvent(ev);
    }
}

void DocumentView::draw()
{
    auto [x, y] = editor.getCaretPosition();
    setCursor(x - delta.x, y - delta.y);
    TDrawSubView::draw();
}
