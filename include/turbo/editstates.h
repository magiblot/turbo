#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#include <tvision/tv.h>
#include <turbo/styles.h>
#include <turbo/funcview.h>
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

    int update(TScintilla &scintilla);

private:

    int calcWidth(TScintilla &scintilla);

};

class WrapState
{
    bool enabled {false};
    bool confirmedOnce {false};

public:

    static bool defConfirmWrap(int width);
    bool toggle(TScintilla &scintilla, TFuncView<bool(int width)> confirmWrap = defConfirmWrap);
};

struct AutoIndent
{
    bool enabled {true};

    void toggle()
    {
        enabled ^= true;
    }

    void applyToCurrentLine(TScintilla &scintilla);
};

class ThemingState
{
public:

    const LexerInfo *lexerInfo {nullptr}; // Non-owning. Lifetime must exceed that of 'this'.
    const ColorScheme *scheme {nullptr}; // Non-owning. Lifetime must exceed that of 'this'.

    // Updates 'scintilla' so that it makes use of the current state of
    // 'lexerInfo' and 'scheme'.
    void apply(TScintilla &scintilla) const;
    // Highlights matching braces if there are any.
    void updateBraces(TScintilla &scintilla) const;

    const ColorScheme &getScheme() const
    {
        return scheme ? *scheme : schemeDefault;
    }
};

void stripTrailingSpaces(TScintilla &scintilla);
void ensureNewlineAtEnd(TScintilla &scintilla);

} // namespace turbo

#endif
