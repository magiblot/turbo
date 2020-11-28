#ifndef TURBO_STYLES_H
#define TURBO_STYLES_H

struct EditorWindow;

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

void loadLexer(Language lang, EditorWindow &win);
void setUpStyles(EditorWindow &win);

class BraceMatching {

public:

    void update(const struct LexerInfo&, Scintilla::TScintillaEditor &editor);

};

class LanguageState {

    const struct LexerInfo *lexInfo {nullptr};
    BraceMatching matching;

public:

    LanguageState() = default;

    LanguageState(const struct LexerInfo *lexInfo) :
        lexInfo(lexInfo)
    {
    }

    void updateBraces(Scintilla::TScintillaEditor &editor)
    {
        if (lexInfo)
            matching.update(*lexInfo, editor);
    }

};

#endif
