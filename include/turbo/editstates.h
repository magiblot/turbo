#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#include <tvision/tv.h>
#include <turbo/styles.h>
#include <turbo/funcview.h>
#include <turbo/scintilla.h>

namespace turbo {

class LineNumbersWidth
{
    int minWidth;
    bool enabled {false};

    int calcWidth(TScintilla &scintilla);

public:

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    inline void setState(bool enable);
    inline void toggle();

    int update(TScintilla &scintilla);
};

inline void LineNumbersWidth::setState(bool enable)
{
    enabled = enable;
}

inline void LineNumbersWidth::toggle()
{
    enabled ^= true;
}

class WrapState
{
    bool enabled {false};
    bool confirmedOnce {false};

public:

    static bool defConfirmWrap(int width);

    // * 'confirmWrap' shall return whether line wrapping should be activated
    //   even if the document is quite large (>= 512 KiB).
    void setState( bool enable, TScintilla &scintilla,
                   TFuncView<bool(int width)> confirmWrap = defConfirmWrap );
    inline void toggle( TScintilla &scintilla,
                        TFuncView<bool(int width)> confirmWrap = defConfirmWrap );
};

inline void WrapState::toggle(TScintilla &scintilla, TFuncView<bool(int width)> confirmWrap)
{
    setState(!enabled, scintilla, confirmWrap);
}

class AutoIndent
{
    bool enabled {true};

public:

    inline void setState(bool enable);
    inline void toggle();

    void applyToCurrentLine(TScintilla &scintilla);
};

inline void AutoIndent::setState(bool enable)
{
    enabled = enable;
}

inline void AutoIndent::toggle()
{
    enabled ^= true;
}

class ThemingState
{
    const LexerInfo *lexerInfo {nullptr};
    const ColorScheme *scheme {nullptr};
public:

    // * 'aLexerInfo': non-owning. Lifetime must exceed that of 'this'.
    inline void setLexerInfo(const LexerInfo *aLexerInfo);
    // * 'aScheme': non-owning. Lifetime must exceed that of 'this'.
    inline void setScheme(const ColorScheme *scheme);
    // Returns the current scheme if present and 'schemeDefault' otherwise.
    inline const ColorScheme &getScheme() const;
    inline bool hasLexer() const;

    // Updates 'scintilla' so that it makes use of the current state of
    // 'lexerInfo' and 'scheme'.
    void apply(TScintilla &scintilla) const;
    // Highlights matching braces if there are any.
    void updateBraces(TScintilla &scintilla) const;
};

inline void ThemingState::setLexerInfo(const LexerInfo *aLexerInfo)
{
    lexerInfo = aLexerInfo;
}

inline void ThemingState::setScheme(const ColorScheme *aScheme)
{
    scheme = aScheme;
}

inline const ColorScheme &ThemingState::getScheme() const
{
    return scheme ? *scheme : schemeDefault;
}

inline bool ThemingState::hasLexer() const
{
    return lexerInfo;
}

void stripTrailingSpaces(TScintilla &scintilla);
void ensureNewlineAtEnd(TScintilla &scintilla);

} // namespace turbo

#endif
