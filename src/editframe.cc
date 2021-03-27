#include "editframe.h"

EditorFrame::EditorFrame(const TRect &bounds) :
    TFrame(bounds),
    editwin(nullptr)
{
}

void EditorFrame::draw()
{
    TFrame::draw();
    if (editwin)
        drawIndicator();
}

void EditorFrame::drawIndicator()
{
    // └─ XXXXXXXXXXXXXX
    //   ^2              ^18
    TRect r(2, size.y - 1, min(size.x - 2, 18), size.y);
    if (r.a.x < r.b.x && r.a.y < r.b.y)
    {
        TDrawBuffer b;
        char s[64];
        auto color = mapColor((state & sfDragging) ? 5 : 4);
        TPoint pos = editwin->cursorPos();
        sprintf(s, " %d:%d ", pos.y + 1, pos.x + 1);
        ushort maxWidth = r.b.x - r.a.x;
        ushort width = b.moveStr(0, s, color, maxWidth);
        ushort left = 5;
        ushort offs = max(left - max(width - (maxWidth - (left + 1)), 0), 0);
        writeLine(r.a.x + offs, r.a.y, min(width, maxWidth), 1, b);
    }
}
