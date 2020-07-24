#ifndef TVEDIT_DRAWVIEWS_H
#define TVEDIT_DRAWVIEWS_H

#define Uses_TView
#include <tvision/tv.h>

#include <vector>

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

class TDrawableView : public TView {

    friend class TDrawSubView;

    std::vector<TScreenCell> drawArea;
    TScreenCell fill;

protected:

    void resizeBuffer();

public:

    TDrawableView(const TRect &bounds);

    virtual void draw() override;
    virtual void changeBounds(const TRect &bounds) override;

    void setFillColor(TCellAttribs fillColor);
    TCellAttribs getFillColor() const;
    TScreenCell& at(int y, int x);
    const TScreenCell& at(int y, int x) const;

};

inline void TDrawableView::setFillColor(TCellAttribs fillColor)
{
    fill.Cell.Attr = fillColor;
    for (TScreenCell &cell : drawArea)
        cell = fill;
}

inline TCellAttribs TDrawableView::getFillColor() const
{
    return fill.Cell.Attr;
}

inline TScreenCell& TDrawableView::at(int y, int x)
{
    return drawArea[y*size.x + x];
}

inline const TScreenCell& TDrawableView::at(int y, int x) const
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
