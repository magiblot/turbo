#ifndef TVEDIT_DRAWVIEW_H
#define TVEDIT_DRAWVIEW_H

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

class TDrawableView : public TView {

    std::vector<TDrawCell> drawArea;

protected:

    void resizeBuffer();

public:

    TDrawableView(const TRect &bounds);

    virtual void draw() override;
    virtual void changeBounds(const TRect &bounds) override;

    TDrawCell& at(int y, int x);
    TRect clipRect(TRect r);

};

inline TDrawCell& TDrawableView::at(int y, int x)
{
    return drawArea[y*size.x + x];
}

inline TRect TDrawableView::clipRect(TRect r)
{
    TRect extent = getExtent();
    extent.intersect(r);
    return extent;
}

#endif
