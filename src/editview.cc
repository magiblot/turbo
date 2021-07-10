#define Uses_TText
#include <tvision/tv.h>

#include <turbo/turbo.h>

namespace turbo {

EditorView::EditorView(const TRect &bounds) :
    TSurfaceView(bounds)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable | ofFirstClick;
    eventMask |= evMouseUp | evMouseMove | evMouseAuto | evBroadcast;
    showCursor();
}

void EditorView::handleEvent(TEvent &ev)
{
    if (!state)
        return;
    TView::handleEvent(ev);
    auto &editor = state->editor;
    switch (ev.what)
    {
        case evKeyDown:
            if (ev.keyDown.keyCode == kbIns)
                setState(sfCursorIns, !getState(sfCursorIns));
            // If we always began reading events in consumeInputText,
            // we would never autoindent newlines.
            if (!ev.keyDown.textLength)
                editor.KeyDownWithModifiers(ev.keyDown, nullptr);
            else
                consumeInputText(ev);
            state->partialRedraw();
            clearEvent(ev);
            break;
        case evMouseDown:
            // Middle button drag
            if (ev.mouse.buttons & mbMiddleButton)
            {
                TPoint lastMouse = makeLocal(ev.mouse.where);
                while (mouseEvent(ev, evMouse))
                {
                    TPoint mouse = makeLocal(ev.mouse.where);
                    TPoint d = editor.getDelta() + (lastMouse - mouse);
                    state->scrollTo(d);
                    state->partialRedraw();
                    lastMouse = mouse;
                }
            }
            else
            {
                // Text selection
                do {
                    TPoint where = makeLocal(ev.mouse.where) + delta;
                    if (ev.what == evMouseWheel)
                    {
                        // Mouse wheel while holding button down.
                        state->scrollBarEvent(ev);
                        ev.mouse.where = where;
                        ev.what = evMouseMove;
                        // For some reason, the caret is not always updated
                        // unless this is invoked twice.
                        editor.MouseEvent(ev);
                        editor.MouseEvent(ev);
                    }
                    else
                    {
                        ev.mouse.where = where;
                        if (!editor.MouseEvent(ev))
                        {
                            state->partialRedraw();
                            break;
                        }
                    }
                    state->partialRedraw();
                } while (mouseEvent(ev, evMouseDown | evMouseMove | evMouseAuto | evMouseWheel));
            }
            clearEvent(ev);
            break;
        case evBroadcast:
            if ( ev.message.command == cmScrollBarChanged &&
                 state->handleScrollBarChanged((TScrollBar *) ev.message.infoPtr) )
            {
                state->partialRedraw();
                clearEvent(ev);
            }
            break;
    }
}

static bool isPastedText(std::string_view text)
{
    size_t i = 0, n = 0;
    while (TText::next(text, i))
        if (++n > 2)
            return true;
    return false;
}

static void insertOneByOne(Editor &editor, std::string_view text)
{
    size_t i = 0, j = 0;
    while (TText::next(text, j))
    {
        // Allow overwrite on Ins.
        editor.insertCharacter(text.substr(i, j));
        i = j;
    }
}

void EditorView::consumeInputText(TEvent &ev)
// Pre: 'state' is non-null.
{
    auto &editor = state->editor;
    editor.clearBeforeTentativeStart();

    char buf[4096];
    size_t length;
    bool undogroup = false;
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
    }

    editor.WndProc(SCI_SCROLLCARET, 0U, 0U);
    if (undogroup)
        editor.WndProc(SCI_ENDUNDOACTION, 0U, 0U);
}

void EditorView::draw()
{
    if (!state)
        TSurfaceView::draw();
    // 'surface' is only set when the draw is triggered by EditorState.
    else if (!surface)
        state->redraw();
    else
    {
        TPoint p = state->editor.getCaretPosition();
        setCursor(p.x - delta.x, p.y - delta.y);
        TSurfaceView::draw();
    }
}

} // namespace turbo
