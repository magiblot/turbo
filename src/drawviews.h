#ifndef TVEDIT_DRAWVIEWS_H
#define TVEDIT_DRAWVIEWS_H

#define Uses_TView
#include <tvision/tv.h>

struct TCellAttribs {

    union {
        struct {
            uchar
                fg : 4,
                bg : 4;
        } colors;
        struct {
            uchar
                fgBlue      : 1,
                fgGreen     : 1,
                fgRed       : 1,
                fgBright    : 1,
                bgBlue      : 1,
                bgGreen     : 1,
                bgRed       : 1,
                bgBright    : 1;

        } bits;
        uchar asChar;
    };

    TCellAttribs(uchar c=0);
    operator uchar&();

};

inline TCellAttribs::TCellAttribs(uchar c) :
    asChar(c)
{
}

inline TCellAttribs::operator uchar&()
{
    return asChar;
}

struct TDrawCell {

    union {
        struct {
            uchar character;
            TCellAttribs attr;
        } cell;
        ushort asShort;
    };

    TDrawCell(ushort s=0);
    operator ushort&();

};

inline TDrawCell::TDrawCell(ushort s) :
    asShort(s)
{
}

inline TDrawCell::operator ushort&()
{
    return asShort;
}

class TDrawSubView;

class TDrawableView : public TView {

    friend class TDrawSubView;

    std::vector<TDrawCell> drawArea;
    TDrawCell fill {0};

protected:

    void resizeBuffer();

public:

    TDrawableView(const TRect &bounds);

    virtual void draw() override;
    virtual void changeBounds(const TRect &bounds) override;

    void setFillColor(TCellAttribs fillColor);
    TCellAttribs getFillColor() const;
    TDrawCell& at(int y, int x);
    const TDrawCell& at(int y, int x) const;

};

inline void TDrawableView::setFillColor(TCellAttribs fillColor)
{
    fill.cell.attr = fillColor;
    for (TDrawCell &cell : drawArea)
        cell = fill;
}

inline TCellAttribs TDrawableView::getFillColor() const
{
    return fill.cell.attr;
}

inline TDrawCell& TDrawableView::at(int y, int x)
{
    return drawArea[y*size.x + x];
}

inline const TDrawCell& TDrawableView::at(int y, int x) const
{
    return drawArea[y*size.x + x];
}

class TDrawSubView : public TView {

protected:

    const TDrawableView &view;
    TPoint delta {0, 0};

public:

    TDrawSubView( const TRect &bounds,
                  const TDrawableView &aView );

    virtual void draw() override;

    void setDelta(TPoint aDelta);

};

inline void TDrawSubView::setDelta(TPoint aDelta)
{
    // Negative delta not supported.
    if (aDelta.x < 0)
        aDelta.x = 0;
    if (aDelta.y < 0)
        aDelta.y = 0;
    delta = aDelta;
}

#endif
