#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#include <tvision/tv.h>
#include <turbo/scintilla.h>

namespace turbo {

class LineNumbersWidth
{

public:

    int minWidth;
    bool enabled {false};

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    void toggle()
    {
        enabled ^= true;
    }

    int update(Scintilla &scintilla);

private:

    int calcWidth(Scintilla &scintilla);

};

bool defWrapIfBig(int width);

class WrapState
{
    bool enabled {false};
    bool confirmedOnce {false};

public:

    bool toggle(Scintilla &scintilla, TFuncView<bool(int)> wrapIfBig = defWrapIfBig);
};

struct AutoIndent
{
    bool enabled {true};

    void toggle()
    {
        enabled ^= true;
    }

    void applyToCurrentLine(Scintilla &scintilla);
};

void stripTrailingSpaces(Scintilla &scintilla);
void ensureNewlineAtEnd(Scintilla &scintilla);

} // namespace turbo

#endif
