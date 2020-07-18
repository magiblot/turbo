#define Uses_TProgram
#include <tvision/tv.h>

#include <ScintillaHeaders.h>

#include "drawviews.h"

using namespace Scintilla;

Window::~Window()
{
}

void Window::Destroy()
{
}

PRectangle Window::GetPosition() const
{
    TDrawableView *v = (TDrawableView *) wid;
    if (v) {
        TRect r = v->getBounds(); // Relative to parent.
        return PRectangle::FromInts(r.a.x, r.a.y, r.b.x, r.b.y);
    }
    return PRectangle();
}

void Window::SetPosition(PRectangle rc)
{
    TDrawableView *v = (TDrawableView *) wid;
    if (v)
        v->changeBounds({{(int) rc.left, (int) rc.top}, {(int) rc.right, (int) rc.bottom}});
}

void Window::SetPositionRelative(PRectangle rc, const Window *relativeTo)
{
    SetPosition(rc);
}

PRectangle Window::GetClientPosition() const
{
    TDrawableView *v = (TDrawableView *) wid;
    if (v) {
        TRect r = v->getExtent(); // r.a is {0, 0}.
        return PRectangle::FromInts(r.a.x, r.a.y, r.b.x, r.b.y);
    }
    return PRectangle();
}

void Window::Show(bool show)
{
    TDrawableView *v = (TDrawableView *) wid;
    if (v) {
        if (show)
            v->show();
        else
            v->hide();
    }
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
