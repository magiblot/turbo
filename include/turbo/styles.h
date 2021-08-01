#ifndef TURBO_STYLES_H
#define TURBO_STYLES_H

#define Uses_TColorAttr
#include <tvision/tv.h>
#include <turbo/scintilla.h>

namespace turbo {

enum Language : uchar
{
    langNone,
    langCPP,
    langMakefile,
    langAsm,
    langJavaScript,
    langRust,
    langPython,
    langBash,
    langDiff,
    langJSON,
    langHTML,
    langXML,
    langVB,
    langPerl,
    langBatch,
    langLaTex,
    langLua,
    langAda,
    langLisp,
    langRuby,
    langTcl,
    langVBScript,
    langMATLAB,
    langCSS,
    langYAML,
    langErlang,
    langSmalltalk,
    langMarkdown,
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

using ColorSchema = TColorAttr[TextStyleCount];

// Returns a color attribute such that:
// * The foreground is taken from 'from' if it is not default, and from 'into' otherwise.
// * The background is taken from 'from' if it is not default, and from 'into' otherwise.
// * The style is taken from 'from'.
TColorAttr coalesce(TColorAttr from, TColorAttr into);

inline TColorAttr normalize(const ColorSchema &schema, TextStyle index)
{
    return coalesce(schema[index], schema[sNormal]);
}

extern const ColorSchema schemaDefault;

struct LexerInfo
{
    struct StyleMapping { uchar id; TextStyle style; };
    struct KeywordMapping { uchar id; const char *keywords; };
    struct PropertyMapping { const char *name, *value; };

    int lexerId;
    TSpan<const StyleMapping> styles;
    TSpan<const KeywordMapping> keywords;
    TSpan<const PropertyMapping> properties;
    TStringView braces;
};

Language detectLanguage(const char *filePath);
const LexerInfo *findLexerInfo(Language language);

} // namespace turbo

#endif
