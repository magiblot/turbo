#ifndef TURBO_EDITSURFACE_H
#define TURBO_EDITSURFACE_H

#define Uses_TDrawSurface
#include <tvision/tv.h>

#include "styles.h"

struct EditorSurface : public TDrawSurface
{

    const ThemingState &theming;

    EditorSurface(TPoint aSize, const ThemingState &aTheming) :
        TDrawSurface(aSize),
        theming(aTheming)
    {
    }

    TColorAttr normalColor()
    {
        return theming.schema[sNormal];
    }

};

#endif // TURBO_EDITSURFACE_H
