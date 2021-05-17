#ifndef TURBO_STYLES_H
#define TURBO_STYLES_H

#include <tvision/tv.h>
#include <utility>

struct BaseEditorWindow;
struct EditorWindow;
struct LexerInfo;
struct TColorAttr;
namespace Scintilla
{
    struct TScintillaEditor;
}

enum Language : unsigned char {
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

enum Styles : unsigned char {
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

typedef TSpan<const std::pair<uchar, Styles>> LexerStyles;
typedef TSpan<const std::pair<uchar, const char *>> LexerKeywords;
typedef TSpan<const std::pair<const char *, const char *>> LexerProperties;

struct ThemingState
{

    const LexerInfo *lexInfo;
    const TColorAttr *schema;

    ThemingState();

    void resetStyles(BaseEditorWindow &win) const;
    void detectLanguage(EditorWindow &win);
    void updateBraces(Scintilla::TScintillaEditor &editor) const;
    TColorAttr normalize(Styles) const;

private:

    void loadLexer(Language lang, BaseEditorWindow &win);
    TColorAttr braceAttr(LexerStyles, uchar) const;

};

#endif
