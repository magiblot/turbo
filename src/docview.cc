#include "docview.h"

using namespace Scintilla;

DocumentView::DocumentView(const TRect &bounds, Scintilla::TScintillaEditor &editor_) :
    TDrawableView(bounds),
    editor(editor_)
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
            break;
        default:
            break;
    }
}

void DocumentView::doUpdate()
{
    auto [x, y] = editor.getCaretPosition();
    setCursor(x, y);
    editor.draw(*this);
    drawView();
}
