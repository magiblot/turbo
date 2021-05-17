#define Uses_TScrollBar
#define Uses_TText
#include <tvision/tv.h>

#include "docview.h"
#include "editwindow.h"

using namespace Scintilla;

DocumentView::DocumentView( const TRect &bounds,
                            const TDrawSurface *view,
                            Scintilla::TScintillaEditor &aEditor,
                            BaseEditorWindow &aWindow ) :
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

static bool isPastedText(std::string_view text)
{
    size_t i = 0, n = 0;
    while (TText::next(text, i))
    {
        if (++n > 2)
            return true;
    }
    return false;
}

static void insertOneByOne( Scintilla::TScintillaEditor &editor,
                            std::string_view text )
{
    size_t i = 0, j = 0;
    while (TText::next(text, j))
    {
        // Allow overwrite on Ins.
        editor.insertCharacter(text.substr(i, j));
        i = j;
    }
}

void DocumentView::consumeInputText(TEvent &ev)
{
    char buf[4096];
    bool undogroup = false;
    size_t length;

    editor.clearBeforeTentativeStart();
    while (textEvent(ev, buf, length))
    {
        std::string_view text {buf, length};
        if (!undogroup && isPastedText(text))
        {
            undogroup = true;
            editor.WndProc(SCI_BEGINUNDOACTION, 0U, 0U);
        }
        if (!undogroup) // Individual typing.
            insertOneByOne(editor, text);
        else
            editor.pasteText(text);
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
