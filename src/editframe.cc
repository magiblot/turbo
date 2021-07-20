#define Uses_TRect
#include <tvision/tv.h>

#include "editframe.h"
#include "tscintilla.h"

EditorFrame::EditorFrame(const TRect &bounds) :
    TFrame(bounds),
    editor(nullptr)
{
}

void EditorFrame::draw()
{
    TFrame::draw();
    if (editor)
        drawIndicator();
}

static TPoint cursorPos(Scintilla::TScintillaEditor &editor)
{
    auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
    return {
        (int) editor.WndProc(SCI_GETCOLUMN, pos, 0U),
        (int) editor.WndProc(SCI_LINEFROMPOSITION, pos, 0U),
    };
}

void EditorFrame::drawIndicator()
// Pre: 'editor != nullptr'.
{
    // └─ XXXXXXXXXXXXXX
    //   ^2              ^18
    TRect r(2, size.y - 1, min(size.x - 2, 18), size.y);
    if (0 <= r.a.x && r.a.x < r.b.x && 0 <= r.a.y && r.a.y < r.b.y)
    {
        TDrawBuffer b;
        char s[64];
        auto color = mapColor((state & sfDragging) ? 5 : 4);
        TPoint pos = cursorPos(*editor);
        sprintf(s, " %d:%d ", pos.y + 1, pos.x + 1);
        ushort maxWidth = r.b.x - r.a.x;
        ushort width = b.moveStr(0, s, color, maxWidth);
        ushort left = 5;
        ushort offs = max(left - max(width - (maxWidth - (left + 1)), 0), 0);
        writeLine(r.a.x + offs, r.a.y, min(width, maxWidth), 1, b);
    }
}
