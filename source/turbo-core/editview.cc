#define Uses_TKeys
#define Uses_TText
#define Uses_TEvent
#include <tvision/tv.h>

#include <turbo/editor.h>

namespace turbo {

EditorView::EditorView(const TRect &bounds) noexcept :
    TSurfaceView(bounds)
{
    growMode = gfGrowHiX | gfGrowHiY;
    options |= ofSelectable | ofFirstClick;
    eventMask |= evMouseUp | evMouseMove | evMouseAuto | evBroadcast;
}

static TPoint getDelta(TScintilla &scintilla)
{
    return {
        (int) call(scintilla, SCI_GETXOFFSET, 0U, 0U),
        (int) call(scintilla, SCI_GETFIRSTVISIBLELINE, 0U, 0U),
    };
}

void EditorView::handleEvent(TEvent &ev)
{
    if (!editor)
        return;
    TView::handleEvent(ev);
    auto &scintilla = editor->scintilla;
    switch (ev.what)
    {
        case evKeyDown:
            if (ev.keyDown.keyCode == kbIns)
                setState(sfCursorIns, !getState(sfCursorIns));
            if (ev.keyDown.controlKeyState & kbPaste)
                handlePaste(ev);
            else
                handleKeyDown(scintilla, ev.keyDown);
            // Could use partialRedraw(), but it is broken for the Redo action (Scintilla bug?).
            editor->redraw();
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
                    TPoint d = getDelta(scintilla) + (lastMouse - mouse);
                    editor->scrollTo(d);
                    editor->partialRedraw();
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
                        editor->scrollBarEvent(ev);
                        ev.mouse.where = where;
                        ev.what = evMouseMove;
                        // For some reason, the caret is not always updated
                        // unless this is invoked twice.
                        handleMouse(scintilla, ev.what, ev.mouse);
                        handleMouse(scintilla, ev.what, ev.mouse);
                    }
                    else
                    {
                        ev.mouse.where = where;
                        if (!handleMouse(scintilla, ev.what, ev.mouse))
                        {
                            editor->partialRedraw();
                            break;
                        }
                    }
                    editor->partialRedraw();
                } while (mouseEvent(ev, evMouseDown | evMouseMove | evMouseAuto | evMouseWheel));
            }
            clearEvent(ev);
            break;
        case evBroadcast:
            if ( ev.message.command == cmScrollBarChanged &&
                 editor->handleScrollBarChanged((TScrollBar *) ev.message.infoPtr) )
            {
                editor->partialRedraw();
                clearEvent(ev);
            }
            break;
    }
}

void EditorView::handlePaste(TEvent &ev)
// Pre: 'editor' is non-null.
{
    auto &scintilla = editor->scintilla;
    clearBeforeTentativeStart(scintilla);
    call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);

    char buf[4096];
    size_t length;
    while (textEvent(ev, buf, length))
    {
        TStringView text(buf, length);
        insertPasteStream(scintilla, text);
    }

    call(scintilla, SCI_SCROLLCARET, 0U, 0U);
    call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
}

void EditorView::draw()
{
    if (!editor)
        TSurfaceView::draw();
    // 'surface' is only set when the draw is triggered by EditorState.
    else if (!surface)
        editor->redraw();
    else
    {
        TPoint p = pointMainCaret(editor->scintilla);
        cursor = p - delta;
        TSurfaceView::draw();
    }
}

} // namespace turbo
