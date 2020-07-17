#include "docview.h"

using namespace Scintilla;

DocumentView::DocumentView(const TRect &bounds, Scintilla::TScintillaEditor &editor_) :
    TDrawableView(bounds),
    editor(editor_)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable;
    setFillColor(0x1E); // Screw palettes, they are too hard to understand.
    editor.setStyleColor(STYLE_DEFAULT, 0x1E);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U);
    editor.setSelectionColor(0x71);
    editor.setWindow(this);
    showCursor();
}

void DocumentView::handleEvent(TEvent &ev)
{
    TView::handleEvent(ev);
    bool handled = true;
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
            handled = false;
    }
    if (handled) {
        clearEvent(ev);
        drawView();
    }
}

void DocumentView::draw()
{
    auto [x, y] = editor.getCaretPosition();
    setCursor(x, y);
    editor.draw(*this);
    TDrawableView::draw();
}
