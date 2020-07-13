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
    return {};
}

void Window::SetPosition(PRectangle rc)
{
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo)
{
}

PRectangle Window::GetClientPosition() const
{
    return {};
}

void Window::Show(bool show)
{
}

void Window::InvalidateAll()
{
}

void Window::InvalidateRectangle(PRectangle rc)
{
}

void Window::SetFont(Font &font)
{
}

void Window::SetCursor(Cursor curs)
{
}

PRectangle Window::GetMonitorRect(Point pt)
{
    return {};
}
