#include <ScintillaHeaders.h>

using namespace Scintilla;

#include "surface.h"

Surface *Surface::Allocate(int technology)
{
    return new TScintillaSurface;
}

void TScintillaSurface::Init(WindowID wid)
{
    view = (TDrawableView *) wid;
}

void TScintillaSurface::Init(SurfaceID sid, WindowID wid)
{
    // We do not distinguish yet between Window and Surface.
    view = (TDrawableView *) wid;
}

void TScintillaSurface::InitPixMap(int width, int height, Surface *surface_, WindowID wid)
{
    view = (TDrawableView *) wid;
}

void TScintillaSurface::Release()
{
    // TScintillaSurface is non-owning.
}

bool TScintillaSurface::Initialised()
{
    return view;
}

void TScintillaSurface::PenColour(ColourDesired fore)
{
}

int TScintillaSurface::LogPixelsY()
{
    return 1;
}

int TScintillaSurface::DeviceHeightFont(int points)
{
    return 1;
}

void TScintillaSurface::MoveTo(int x_, int y_)
{
}

void TScintillaSurface::LineTo(int x_, int y_)
{
}

void TScintillaSurface::Polygon(Point *pts, size_t npts, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::FillRectangle(PRectangle rc, ColourDesired back)
{
    // Used to draw text selections. Do not overwrite the foreground color.
    auto r = clipRect(rc);
    uchar bg = convertColor(back);
    uchar fg = view->getFillColor().colors.fg;
    for (int y = r.a.y; y < r.b.y; ++y)
        for (int x = r.a.x; x < r.b.x; ++x) {
            auto c = view->at(y, x);
            c.cell.attr.colors.bg = bg;
            c.cell.attr.colors.fg = fg;
            c.cell.character = '\0';
            view->at(y, x) = c;
        }
}

void TScintillaSurface::FillRectangle(PRectangle rc, Surface &surfacePattern)
{
    FillRectangle(rc, ColourDesired());
}

void TScintillaSurface::RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
        ColourDesired outline, int alphaOutline, int flags)
{
    auto r = clipRect(rc);
    TCellAttribs attr = convertColorPair(outline, fill);
    for (int y = r.a.y; y < r.b.y; ++y)
        for (int x = r.a.x; x < r.b.x; ++x)
            view->at(y, x).cell.attr = attr;
}

void TScintillaSurface::GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options)
{
}

void TScintillaSurface::DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage)
{
}

void TScintillaSurface::Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back)
{
}

void TScintillaSurface::Copy(PRectangle rc, Point from, Surface &surfaceSource)
{
}

std::unique_ptr<IScreenLineLayout> TScintillaSurface::Layout(const IScreenLine *screenLine)
{
    return nullptr;
}

void TScintillaSurface::DrawTextNoClip( PRectangle rc, Font &font_,
                                        XYPOSITION ybase, std::string_view text,
                                        ColourDesired fore, ColourDesired back )
{
    auto clip_ = clip;
    clip = view->getExtent();
    DrawTextClipped(rc, font_, ybase, text, fore, back);
    clip = clip_;
}

void TScintillaSurface::DrawTextClipped( PRectangle rc, Font &font_,
                                         XYPOSITION ybase, std::string_view text,
                                         ColourDesired fore, ColourDesired back )
{
    auto r = clipRect(rc);
    uchar color = convertColorPair(fore, back);
    for (int y = r.a.y; y < r.b.y; ++y) {
        int x = r.a.x;
        int i = 0;
        int count = std::min(r.b.x - r.a.x, (int) text.size());
        for (; i < count; ++i, ++x) {
            auto c = view->at(y, x);
            c.cell.attr = color;
            c.cell.character = (uchar) text[i];
            view->at(y, x) = c;
        }
    }
}

void TScintillaSurface::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore)
{
    auto r = clipRect(rc);
    uchar fg = convertColor(fore);
    for (int y = r.a.y; y < r.b.y; ++y) {
        int x = r.a.x;
        int i = 0;
        int count = std::min(r.b.x - r.a.x, (int) text.size());
        for (; i < count; ++i, ++x) {
            auto c = view->at(y, x);
            c.cell.attr.colors.fg = fg;
            c.cell.character = (uchar) text[i];
            view->at(y, x) = c;
        }
    }
}

void TScintillaSurface::MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions)
{
    for (size_t i = 0; i < text.size(); ++i)
        positions[i] = i + 1;
}

XYPOSITION TScintillaSurface::WidthText(Font &font_, std::string_view text)
{
    return text.size();
}

XYPOSITION TScintillaSurface::Ascent(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::Descent(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::InternalLeading(Font &font_)
{
    return 0;
}

XYPOSITION TScintillaSurface::Height(Font &font_)
{
    return 1;
}

XYPOSITION TScintillaSurface::AverageCharWidth(Font &font_)
{
    return 1;
}

void TScintillaSurface::SetClip(PRectangle rc)
{
    clip = rc;
    clip.intersect(view->getExtent());
}

void TScintillaSurface::FlushCachedState()
{
}

void TScintillaSurface::SetUnicodeMode(bool unicodeMode_)
{
}

void TScintillaSurface::SetDBCSMode(int codePage)
{
}

void TScintillaSurface::SetBidiR2L(bool bidiR2L_)
{
}
