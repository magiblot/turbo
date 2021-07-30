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

enum Styles : uchar
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
    sFramePassive,
    sFrameActive,
    sFrameIcon,
    sStaticText,
    sLabelNormal,
    sLabelSelected,
    sLabelShortcut,
    sButtonNormal,
    sButtonDefault,
    sButtonSelected,
    sButtonDisabled,
    sButtonShortcut,
    sButtonShadow,
    StyleCount,
};

using ColorSchema = TColorAttr[StyleCount];

// Returns a color attribute such that:
// * The foreground is taken from 'from' if it is not default, and from 'into' otherwise.
// * The background is taken from 'from' if it is not default, and from 'into' otherwise.
// * The style is taken from 'from'.
TColorAttr coalesce(TColorAttr from, TColorAttr into);

inline TColorAttr normalize(const ColorSchema &schema, Styles index)
{
    return coalesce(schema[index], schema[sNormal]);
}

extern const ColorSchema schemaDefault;

struct LexerInfo
{
    struct StyleMapping { uchar id; Styles style; };
    struct KeywordMapping { uchar id; const char *keywords; };
    struct PropertyMapping { const char *name, *value; };

    int lexerId;
    TSpan<const StyleMapping> styles;
    TSpan<const KeywordMapping> keywords;
    TSpan<const PropertyMapping> properties;
    TStringView braces;
};

Language detectLanguage(const char *filePath);
const LexerInfo *findLexer(Language language);

class ThemingState
{
public:

    const LexerInfo *lexInfo {nullptr};
    const ColorSchema *schema {nullptr};

    // Modifying 'lexInfo' or 'schema' does not have any effects until this
    // is invoked.
    void apply(TScintilla &scintilla) const;
    void updateBraces(TScintilla &scintilla) const;

    const ColorSchema &getSchema() const
    {
        return schema ? *schema : schemaDefault;
    }
};

} // namespace turbo

#endif
