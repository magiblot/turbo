#define Uses_TScrollBar
#include <tvision/tv.h>

#include "docview.h"
#include "editwindow.h"

using namespace Scintilla;

DocumentView::DocumentView( const TRect &bounds,
                            const TDrawSurface *view,
                            Scintilla::TScintillaEditor &aEditor,
                            EditorWindow &aWindow ) :
    TSurfaceView(bounds, view),
    editor(aEditor),
    window(aWindow)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable | ofFirstClick;
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
            // If we always began reading events in consumeInputText,
            // we would never autoindent newlines.
            if (!ev.keyDown.textLength)
                editor.KeyDownWithModifiers(ev.keyDown, nullptr);
            else
                consumeInputText(ev);
            handled = true;
            break;
        case evMouseDown:
            // Middle button drag
            if (ev.mouse.buttons & mbMiddleButton) {
                TPoint lastMouse = makeLocal(ev.mouse.where);
                while (mouseEvent(ev, evMouse)) {
                    TPoint mouse = makeLocal(ev.mouse.where);
                    TPoint d = editor.getDelta() + (lastMouse - mouse);
                    window.scrollTo(d);
                    window.redrawEditor();
                    lastMouse = mouse;
                }
                handled = true;
                break;
            }
            // Text selection
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
        default:
            handled = false;
    }
    if (handled) {
        window.redrawEditor();
        clearEvent(ev);
    }
}

void DocumentView::consumeInputText(TEvent &ev)
{
    char buf[4096];
    bool undogroup = false;
    size_t count = 0, size;

    while (textEvent(ev, buf, size, count)) {
        if (!undogroup && count > 2) {
            undogroup = true;
            editor.WndProc(SCI_BEGINUNDOACTION, 0U, 0U);
        }
        editor.pasteText({buf, size});
    };

    editor.WndProc(SCI_SCROLLCARET, 0U, 0U);
    if (undogroup)
        editor.WndProc(SCI_ENDUNDOACTION, 0U, 0U);
}

void DocumentView::draw()
{
    auto [x, y] = editor.getCaretPosition();
    setCursor(x - delta.x, y - delta.y);
    TSurfaceView::draw();
}
