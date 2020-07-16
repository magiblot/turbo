#ifndef TVEDIT_SURFACE_H
#define TVEDIT_SURFACE_H

#define Uses_TPoint
#define Uses_TRect
#include <tvision/tv.h>

#include <ScintillaHeaders.h>

class TDrawableView;
struct TDrawCell;
struct TCellAttribs;

namespace Scintilla {

    struct TScintillaSurface : public Surface {

        TDrawableView *view {0};

        static TDrawCell makeCell(uchar ch, ColourDesired fore, ColourDesired back);
        static TCellAttribs convertColor(ColourDesired c);
        static TCellAttribs convertColorPair(ColourDesired fore, ColourDesired back);
        static ColourDesired convertColor(uchar c);

        void Init(WindowID wid) override;
        void Init(SurfaceID sid, WindowID wid) override;
        void InitPixMap(int width, int height, Surface *surface_, WindowID wid) override;

        void Release() override;
        bool Initialised() override;
        void PenColour(ColourDesired fore) override;
        int LogPixelsY() override;
        int DeviceHeightFont(int points) override;
        void MoveTo(int x_, int y_) override;
        void LineTo(int x_, int y_) override;
        void Polygon(Point *pts, size_t npts, ColourDesired fore, ColourDesired back) override;
        void RectangleDraw(PRectangle rc, ColourDesired fore, ColourDesired back) override;
        void FillRectangle(PRectangle rc, ColourDesired back) override;
        void FillRectangle(PRectangle rc, Surface &surfacePattern) override;
        void RoundedRectangle(PRectangle rc, ColourDesired fore, ColourDesired back) override;
        void AlphaRectangle(PRectangle rc, int cornerSize, ColourDesired fill, int alphaFill,
                ColourDesired outline, int alphaOutline, int flags) override;
        void GradientRectangle(PRectangle rc, const std::vector<ColourStop> &stops, GradientOptions options) override;
        void DrawRGBAImage(PRectangle rc, int width, int height, const unsigned char *pixelsImage) override;
        void Ellipse(PRectangle rc, ColourDesired fore, ColourDesired back) override;
        void Copy(PRectangle rc, Point from, Surface &surfaceSource) override;

        std::unique_ptr<IScreenLineLayout> Layout(const IScreenLine *screenLine) override;

        void DrawTextNoClip(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) override;
        void DrawTextClipped(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore, ColourDesired back) override;
        void DrawTextTransparent(PRectangle rc, Font &font_, XYPOSITION ybase, std::string_view text, ColourDesired fore) override;
        void MeasureWidths(Font &font_, std::string_view text, XYPOSITION *positions) override;
        XYPOSITION WidthText(Font &font_, std::string_view text) override;
        XYPOSITION Ascent(Font &font_) override;
        XYPOSITION Descent(Font &font_) override;
        XYPOSITION InternalLeading(Font &font_) override;
        XYPOSITION Height(Font &font_) override;
        XYPOSITION AverageCharWidth(Font &font_) override;

        void SetClip(PRectangle rc) override;
        void FlushCachedState() override;

        void SetUnicodeMode(bool unicodeMode_) override;
        void SetDBCSMode(int codePage) override;
        void SetBidiR2L(bool bidiR2L_) override;

    };

}

#include "drawview.h"

namespace Scintilla {

    inline TDrawCell TScintillaSurface::makeCell(uchar ch, ColourDesired fore, ColourDesired back)
    {
        uchar color = convertColorPair(fore, back);
        return ushort ((color << 8) | ch);
    }

    inline TCellAttribs TScintillaSurface::convertColor(ColourDesired c)
    {
        TCellAttribs attr {};
        attr.bits.fgBlue = c.GetBlue() > 0x55;
        attr.bits.fgGreen = c.GetGreen() > 0x55;
        attr.bits.fgRed = c.GetRed() > 0x55;
        attr.bits.fgBright = c.GetBlue() > 0xAA || c.GetGreen() > 0xAA || c.GetRed() > 0xAA;
        return attr;
    }

    inline TCellAttribs TScintillaSurface::convertColorPair(ColourDesired fore, ColourDesired back)
    {
        TCellAttribs attr = (convertColor(back) << 4) | convertColor(fore);
        if (attr.colors.fg == attr.colors.bg && !(fore == back)) {
            uint grayFg = (fore.GetBlue() + fore.GetGreen() + fore.GetRed())/3;
            uint grayBg = (back.GetBlue() + back.GetGreen() + back.GetRed())/3;
            if (grayFg < grayBg) {
                attr.bits.fgBright = 0;
                attr.bits.bgBright = 1;
            } else {
                attr.bits.fgBright = 1;
                attr.bits.bgBright = 0;
            }
        }
        return attr;
    }

    inline __attribute__((noinline)) ColourDesired TScintillaSurface::convertColor(uchar c)
    {
        TCellAttribs attr {c};
        uchar red = attr.bits.fgRed ? attr.bits.fgBright ? 0xFF : 0x7F : 0x00;
        uchar green = attr.bits.fgGreen ? attr.bits.fgBright ? 0xFF : 0x7F : 0x00;
        uchar blue = attr.bits.fgBlue ? attr.bits.fgBright ? 0xFF : 0x7F : 0x00;
        return ColourDesired(red, green, blue);
    }

}

#endif
