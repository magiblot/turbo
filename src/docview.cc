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
    showCursor();
}

void DocumentView::handleEvent(TEvent &ev)
{
    TView::handleEvent(ev);
    switch (ev.what) {
        case evKeyDown:
            switch (ev.keyDown.keyCode) {
                case kbIns:
                    setState(sfCursorIns, Boolean(!getState(sfCursorIns)));
                    break;
            }
            editor.KeyDownWithModifiers(ev.keyDown, nullptr);
            window.redrawEditor();
            clearEvent(ev);
            break;
        default:
            break;
    }
}


void DocumentView::changeBounds(const TRect &bounds)
{
    TDrawableView::changeBounds(bounds);
    window.redrawEditor();
    // If redrawEditor() did no changes to margin sizes, the following
    // would have to be called first:
//     editor.changeSize();
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
