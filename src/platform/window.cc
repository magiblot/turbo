#define Uses_TProgram
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <ScintillaHeaders.h>
#include "../tscintilla.h"
#include "surface.h"

using namespace Scintilla;

Window::~Window()
{
}

void Window::Destroy()
{
}

PRectangle Window::GetPosition() const
{
    return PRectangle();
}

void Window::SetPosition(PRectangle rc)
{
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo)
{
}

PRectangle Window::GetClientPosition() const
{
    auto *p = (TScintillaParent *) wid;
    if (p)
    {
        auto size = p->getEditorSize();
        return PRectangle::FromInts(0, 0, size.x, size.y);
    }
    return PRectangle();
}

void Window::Show(bool show)
{
}

void Window::InvalidateAll()
{
}

void Window::InvalidateRectangle(PRectangle rc)
{
    // Partial redraw is not supported.
    InvalidateAll();
}

void Window::SetFont(Font &font)
{
}

void Window::SetCursor(Cursor curs)
{
}

PRectangle Window::GetMonitorRect(Point pt)
{
    if (TProgram::application)
    {
        auto size = TProgram::application->size;
        return PRectangle::FromInts(0, 0, size.x, size.y);
    }
    return PRectangle();
}
