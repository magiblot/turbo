#define Uses_TProgram
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <ScintillaHeaders.h>

using namespace Scintilla;

Window::~Window()
{
}

void Window::Destroy()
{
}

PRectangle Window::GetPosition() const
{
    auto *v = (TDrawSurface *) wid;
    if (v)
        return PRectangle::FromInts(0, 0, v->size.x, v->size.y);
    return PRectangle();
}

void Window::SetPosition(PRectangle rc)
{
    auto *v = (TDrawSurface *) wid;
    if (v)
        v->resize({int(rc.right - rc.left), int(rc.bottom - rc.top)});
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo)
{
    SetPosition(rc);
}

PRectangle Window::GetClientPosition() const
{
    auto *v = (TDrawSurface *) wid;
    if (v)
        return PRectangle::FromInts(0, 0, v->size.x, v->size.y);
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
    if (TProgram::application) {
        TPoint r = TProgram::application->size;
        return PRectangle::FromInts(0, 0, r.x, r.y);
    }
    return PRectangle();
}
