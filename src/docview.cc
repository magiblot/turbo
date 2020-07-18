#define Uses_TScrollBar
#include <tvision/tv.h>

#include "docview.h"
#include "editwindow.h"

using namespace Scintilla;

DocumentView::DocumentView( const TRect &bounds,
                            Scintilla::TScintillaEditor &aEditor,
                            EditorWindow &aWindow ) :
    TDrawableView(bounds),
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
                if (ev.what == evMouseWheel) {
                    // Mouse wheel while holding button down.
                    window.scrollBarEvent(ev);
                    ev.mouse.where = makeLocal(ev.mouse.where);
                    ev.what = evMouseMove;
                    // For some reason, the caret is not always updated
                    // unless this is invoked twice.
                    editor.MouseEvent(ev);
                    editor.MouseEvent(ev);
                } else {
                    ev.mouse.where = makeLocal(ev.mouse.where);
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

void DocumentView::setState(ushort aState, Boolean enable)
{
    TView::setState(aState, enable);
    switch (aState) {
        // We handle this here as the lifetime of DocumentView
        // never exceeds that of EditorWindow.
        case sfActive:
            window.setActive(enable);
            break;
    }
}

void DocumentView::doUpdate()
{
    editor.draw(*this);
    // It is important to draw first. Otherwise, the caret position could
    // be affected by margins present in the editor's previous last draw.
    auto [x, y] = editor.getCaretPosition();
    setCursor(x, y);
    drawView();
}
