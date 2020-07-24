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
            c.Cell.Attr.colors.bg = bg;
            c.Cell.Attr.colors.fg = fg;
            c.Cell.Char.asInt = '\0';
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
            view->at(y, x).Cell.Attr = attr;
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
    auto color = convertColorPair(fore, back);
    std::mbstate_t state {};
    size_t skipChars = 0;
    utf8advance(text, skipChars, clip.a.x - (int) rc.left, state);
    size_t textBegin = skipChars;
    for (int y = r.a.y; y < r.b.y; ++y) {
        int x = r.a.x;
        size_t i = textBegin;
        while (i < text.size() && x < r.b.x) {
            auto &c = view->at(y, x);
            c.Cell.Attr = color;
            utf8read(&c, r.b.x - x, text.substr(i, text.size() - i), i, x, state);
        }
    }
}

void TScintillaSurface::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore)
{
    auto r = clipRect(rc);
    uchar fg = convertColor(fore);
    std::mbstate_t state {};
    size_t skipChars = 0;
    utf8advance(text, skipChars, clip.a.x - (int) rc.left, state);
    size_t textBegin = skipChars;
    for (int y = r.a.y; y < r.b.y; ++y) {
        int x = r.a.x;
        size_t i = textBegin;
        while (i < text.size() && x < r.b.x) {
            auto &c = view->at(y, x);
            c.Cell.Attr.colors.fg = fg;
            utf8read(&c, r.b.x - x, text.substr(i, text.size() - i), i, x, state);
        }
    }
}

void TScintillaSurface::MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions)
{
    std::mbstate_t state {};
    size_t i = 0;
    int j = 1;
    while (i < text.size()) {
        int width = 0;
        size_t len = 0;
        utf8len({&text[i], text.size() - i}, len, width, state);
        while (len--)
            positions[i++] = j;
        j += width;
    }
}

XYPOSITION TScintillaSurface::WidthText(Font &font_, std::string_view text)
{
    std::mbstate_t state {};
    size_t i = 0;
    int j = 0;
    while (i < text.size())
        utf8len({&text[i], text.size() - i}, i, j, state);
    return j;
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
