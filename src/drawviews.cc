#include "drawviews.h"

TDrawableView::TDrawableView(const TRect &bounds) :
    TView(bounds),
    drawArea(size.x*size.y),
    fill(0)
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
    drawArea.resize(size.x*size.y, fill);
}

TDrawSubView::TDrawSubView( const TRect &bounds,
                            const TDrawableView &aView ) :
    TView(bounds),
    view(aView)
{
}

void TDrawSubView::draw()
{
    int countX = min(size.x, view.size.x - delta.x);
    int countY = min(size.y, view.size.y - delta.y);
    if (countX == size.x)
        for (int y = 0; y < countY; ++y)
            writeBuf(0, y, countX, 1, &view.at(y + delta.y, delta.x));
    else { // countX < size.x
        uchar color = view.getFillColor();
        for (int y = 0; y < countY; ++y) {
            writeBuf(0, y, countX, 1, &view.at(y + delta.y, delta.x));
            writeChar(countX, y, '\0', color, size.x - countX);
        }
    }
}

