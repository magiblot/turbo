#include "drawview.h"

TDrawableView::TDrawableView(const TRect &bounds) :
    TView(bounds),
    drawArea(size.x*size.y)
{
}

void TDrawableView::draw()
{
    writeBuf(0, 0, size.x, size.y, drawArea.data());
}

void TDrawableView::changeBounds(const TRect &bounds)
{
    setBounds(bounds);
    resizeBuffer();
    drawView();
}

void TDrawableView::resizeBuffer()
{
    drawArea.resize(size.x*size.y);
}
