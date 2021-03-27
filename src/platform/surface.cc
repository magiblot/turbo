#include <ScintillaHeaders.h>

using namespace Scintilla;

#define Uses_TText
#include <tvision/tv.h>

#include "surface.h"
#include "../editsurface.h"

Surface *Surface::Allocate(int technology)
{
    return new TScintillaSurface;
}

void TScintillaSurface::Init(WindowID wid)
{
    view = (EditorSurface *) wid;
}

void TScintillaSurface::Init(SurfaceID sid, WindowID wid)
{
    // We do not distinguish yet between Window and Surface.
    view = (EditorSurface *) wid;
}

void TScintillaSurface::InitPixMap(int width, int height, Surface *surface_, WindowID wid)
{
    view = (EditorSurface *) wid;
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
    // Used to draw text selections and areas without text. The foreground color
    // also needs to be set or else the cursor will have the wrong color when
    // placed on this area.
    auto r = clipRect(rc);
    auto attr = view->normalColor();
    ::setBack(attr, convertColor(back));
    TScreenCell cell;
    ::setCell(cell, ' ', attr);
    for (int y = r.a.y; y < r.b.y; ++y)
        for (int x = r.a.x; x < r.b.x; ++x)
            view->at(y, x) = cell;
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
    auto fg = convertColor(outline),
         bg = convertColor(fill);
    for (int y = r.a.y; y < r.b.y; ++y)
        for (int x = r.a.x; x < r.b.x; ++x) {
            auto &attr = view->at(y, x).attr;
            ::setFore(attr, fg);
            ::setBack(attr, bg);
        }
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
    clip = {0, 0, view->size.x, view->size.y};
    DrawTextClipped(rc, font_, ybase, text, fore, back);
    clip = clip_;
}

void TScintillaSurface::DrawTextClipped( PRectangle rc, Font &font_,
                                         XYPOSITION ybase, std::string_view text,
                                         ColourDesired fore, ColourDesired back )
{
    auto r = clipRect(rc);
    auto attr = convertColorPair(fore, back);
    ::setStyle(attr, getStyle(font_));
    size_t textBegin = 0, overlap = 0;
    TText::wseek(text, textBegin, overlap, clip.a.x - (int) rc.left);
    for (int y = r.a.y; y < r.b.y; ++y) {
        auto cells = TSpan<TScreenCell>(&view->at(y, 0), r.b.x);
        size_t x = r.a.x;
        while (overlap-- && (int) x < r.b.x)
            ::setCell(cells[x++], ' ', attr);
        TText::fill(cells.subspan(x), text.substr(textBegin), attr);
    }
}

void TScintillaSurface::DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore)
{
    auto r = clipRect(rc);
    auto fg = convertColor(fore);
    auto style = getStyle(font_);
    size_t textBegin = 0, overlap = 0;
    TText::wseek(text, textBegin, overlap, clip.a.x - (int) rc.left);
    for (int y = r.a.y; y < r.b.y; ++y) {
        auto cells = TSpan<TScreenCell>(&view->at(y, 0), r.b.x);
        size_t x = r.a.x;
        for (; overlap-- && (int) x < r.b.x; ++x) {
            auto &c = cells[x];
            ::setFore(c.attr, fg);
            ::setStyle(c.attr, style);
            c.ch = ' ';
        }
        TText::fill(cells.subspan(x), text.substr(textBegin),
            [fg, style] (auto &attr) {
                ::setFore(attr, fg);
                ::setStyle(attr, style);
            }
        );
    }
}

void TScintillaSurface::MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions)
{
    size_t i = 0, j = 1;
    while (i < text.size()) {
        size_t width = 0, k = i;
        TText::next(text, i, width);
        // I don't know why. It just works.
        j += width - 1;
        while (k < i)
            positions[k++] = (int) j;
        ++j;
    }
}

XYPOSITION TScintillaSurface::WidthText(Font &font_, std::string_view text)
{
    return strwidth(text);
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
    clip.intersect({0, 0, view->size.x, view->size.y});
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
