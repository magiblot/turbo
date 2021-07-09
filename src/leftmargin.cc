#include <turbo/turbo.h>

namespace turbo {

LeftMarginView::LeftMarginView(int aDistance) :
    TSurfaceView(TRect(0, 0, 0, 0)),
    distanceFromView(aDistance)
{
    growMode = gfGrowHiY | gfFixed;
}

} // namespace turbo
