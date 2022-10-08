#ifndef TURBO_STYLES_H
#define TURBO_STYLES_H

#define Uses_TColorAttr
#include <tvision/tv.h>
#include <turbo/scintilla.h>

namespace turbo {

class Language
{
public:

    static const Language
        CPP,
        Makefile,
        Asm,
        JavaScript,
        Rust,
        Python,
        Bash,
        Diff,
        JSON,
        HTML,
        XML,
        VB,
        Perl,
        Batch,
        LaTex,
        Lua,
        Ada,
        Lisp,
        Ruby,
        Tcl,
        VBScript,
        MATLAB,
        CSS,
        YAML,
        Erlang,
        Smalltalk,
        Markdown;

private:

    Language() = default;
};

enum TextStyle : uchar
{
    sNormal,
    sSelection,
    sWhitespace,
    sCtrlChar,
    sLineNums,
    sKeyword1,
    sKeyword2,
    sMisc,
    sPreprocessor,
    sOperator,
    sComment,
    sStringLiteral,
    sCharLiteral,
    sNumberLiteral,
    sEscapeSequence,
    sError,
    sBraceMatch,
    TextStyleCount,
};

using ColorScheme = TColorAttr[TextStyleCount];

// Returns a color attribute such that:
// * The foreground is taken from 'from' if it is not default, and from 'into' otherwise.
// * The background is taken from 'from' if it is not default, and from 'into' otherwise.
// * The style is taken from 'from'.
TColorAttr coalesce(TColorAttr from, TColorAttr into);

inline TColorAttr normalize(const ColorScheme &scheme, TextStyle index)
{
    return coalesce(scheme[index], scheme[sNormal]);
}

extern const ColorScheme schemeDefault;

struct LexerSettings
{
    struct StyleMapping { uchar id; TextStyle style; };
    struct KeywordMapping { uchar id; const char *keywords; };
    struct PropertyMapping { const char *name, *value; };

    int id;
    TSpan<const StyleMapping> styles;
    TSpan<const KeywordMapping> keywords;
    TSpan<const PropertyMapping> properties;
};

const Language *detectFileLanguage(const char *filePath);
const LexerSettings *findBuiltInLexer(const Language *language);

} // namespace turbo

#endif
