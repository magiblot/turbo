#ifndef TURBO_STYLES_H
#define TURBO_STYLES_H

struct EditorWindow;
struct LexerInfo;

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

// To initialize styling in an EditorWindow:
// 1. '::setUpStyles(win)' initializes the language-independent styles.
// 2. 'LanguageState::detect(win)' detects the file type and initializes lexing
//    and language-aware styles.

void setUpStyles(EditorWindow &win);

class BraceMatching {

public:

    void update(const LexerInfo&, Scintilla::TScintillaEditor &editor);

};

class LanguageState {

    const LexerInfo *lexInfo {nullptr};
    BraceMatching matching;

    void loadLexer(Language lang, EditorWindow &win);

public:

    LanguageState() = default;

    LanguageState(const LexerInfo *lexInfo) :
        lexInfo(lexInfo)
    {
    }

    void detect(EditorWindow &win);

    void updateBraces(Scintilla::TScintillaEditor &editor)
    {
        if (lexInfo)
            matching.update(*lexInfo, editor);
    }

};

#endif
