#include <tvision/tv.h>
#include "util.h"
#include "editwindow.h"
#include "tscintilla.h"
#include <utility>
using namespace Scintilla;
using std::pair;

enum Styles : uchar {
    sNormal,
    sSelection,
    sWhitespace,
    sCtrlChar,
    sLineNums,
    sKeyword1,
    sKeyword2,
    sGlobals,
    sPreprocessor,
    sOperator,
    sComment,
    sStringLiteral,
    sCharLiteral,
    sNumberLiteral,
    sEscapeSequence,
};

static const TCellAttribs styleDefaults[] = {
    [sNormal]         = {0x07, afFgDefault | afBgDefault    },
    [sSelection]      = {0x71                               },
    [sWhitespace]     = {0x05, afBgDefault                  },
    [sCtrlChar]       = {0x0D, afBgDefault                  },
    [sLineNums]       = {0x06, afBgDefault                  },
    [sKeyword1]       = {0x0E, afBgDefault                  },
    [sKeyword2]       = {0x0A, afBgDefault                  },
    [sGlobals]        = {0x09, afBgDefault                  },
    [sPreprocessor]   = {0x02, afBgDefault                  },
    [sOperator]       = {0x0D, afBgDefault                  },
    [sComment]        = {0x06, afBgDefault                  },
    [sStringLiteral]  = {0x0C, afBgDefault                  },
    [sCharLiteral]    = {0x0C, afBgDefault                  },
    [sNumberLiteral]  = {0x03, afBgDefault                  },
    [sEscapeSequence] = {0x0B, afBgDefault                  },
};

static constexpr pair<uchar, Styles> stylesC[] = {
    {SCE_C_DEFAULT,                 sNormal},
    {SCE_C_COMMENT,                 sComment},
    {SCE_C_COMMENTLINE,             sComment},
    {SCE_C_COMMENTDOC,              sComment},
    {SCE_C_NUMBER,                  sNumberLiteral},
    {SCE_C_WORD,                    sKeyword1},
    {SCE_C_STRING,                  sStringLiteral},
    {SCE_C_CHARACTER,               sCharLiteral},
    {SCE_C_PREPROCESSOR,            sPreprocessor},
    {SCE_C_OPERATOR,                sOperator},
    {SCE_C_COMMENTLINEDOC,          sComment},
    {SCE_C_WORD2,                   sKeyword2},
    {SCE_C_GLOBALCLASS,             sGlobals},
    {SCE_C_PREPROCESSORCOMMENT,     sComment},
    {SCE_C_PREPROCESSORCOMMENTDOC,  sComment},
    {SCE_C_ESCAPESEQUENCE,          sEscapeSequence},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, const char *> keywordsC[] = {
    {0,
"alignas alignof and and_eq asm auto bitand bitor break case catch class compl "
"concept consteval constexpr constinit const_cast continue co_await co_return "
"co_yield decltype default delete do dynamic_cast else enum explicit export "
"false final for friend goto if import inline module namespace new noexcept not "
"not_eq nullptr operator or or_eq override private protected public "
"reinterpret_cast return sizeof static_assert static_cast struct switch "
"template this throw true try typedef typeid typename union using virtual while "
"xor xor_eq "
    },
    {1,
"bool char char8_t char16_t char32_t const double extern float int long mutable "
"register static short signed unsigned thread_local void volatile wchar_t int8_t "
"uint8_t int16_t uint16_t int32_t uint32_t int64_t uint64_t size_t ptrdiff_t "
"intptr_t uintptr_t far near uchar ushort uint ulong "
    },
    {3,
"std"
    },
    {(uchar) -1, nullptr},
};

static constexpr pair<const char *, const char *> propertiesC[] = {
    {"styling.within.preprocessor",         "1"},
    {"lexer.cpp.track.preprocessor",        "0"},
    {"lexer.cpp.escape.sequence",           "1"},
    {nullptr, nullptr},
};


static constexpr pair<uchar, Styles> stylesMake[] = {
    {SCE_MAKE_DEFAULT,              sNormal},
    {SCE_MAKE_COMMENT,              sComment},
    {SCE_MAKE_TARGET,               sKeyword1},
    {SCE_MAKE_IDENTIFIER,           sPreprocessor},
    {SCE_MAKE_PREPROCESSOR,         sPreprocessor},
    {SCE_MAKE_OPERATOR,             sOperator},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, Styles> stylesAsm[] = {
    {SCE_ASM_DEFAULT,               sNormal},
    {SCE_ASM_COMMENT,               sComment},
    {SCE_ASM_COMMENTBLOCK,          sComment},
    {SCE_ASM_NUMBER,                sNumberLiteral},
    {SCE_ASM_CPUINSTRUCTION,        sKeyword1},
    {SCE_ASM_MATHINSTRUCTION,       sKeyword1},
    {SCE_ASM_STRING,                sStringLiteral},
    {SCE_ASM_CHARACTER,             sCharLiteral},
    {SCE_ASM_DIRECTIVE,             sPreprocessor},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, Styles> stylesCoffeeScript[] = {
    {SCE_C_DEFAULT,                 sNormal},
    {SCE_COFFEESCRIPT_COMMENT,      sComment},
    {SCE_COFFEESCRIPT_COMMENTLINE,  sComment},
    {SCE_COFFEESCRIPT_COMMENTDOC,   sComment},
    {SCE_COFFEESCRIPT_NUMBER,       sNumberLiteral},
    {SCE_COFFEESCRIPT_WORD,         sKeyword1},
    {SCE_COFFEESCRIPT_STRING,       sStringLiteral},
    {SCE_COFFEESCRIPT_CHARACTER,    sCharLiteral},
    {SCE_COFFEESCRIPT_PREPROCESSOR, sPreprocessor},
    {SCE_COFFEESCRIPT_OPERATOR,     sOperator},
    {SCE_COFFEESCRIPT_COMMENTLINEDOC,sComment},
    {SCE_COFFEESCRIPT_WORD2,        sKeyword2},
    {SCE_COFFEESCRIPT_GLOBALCLASS,  sGlobals},
    {SCE_COFFEESCRIPT_COMMENTBLOCK, sComment},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, Styles> stylesRust[] = {
    {SCE_RUST_DEFAULT,              sNormal},
    {SCE_RUST_COMMENTBLOCK,         sComment},
    {SCE_RUST_COMMENTLINE,          sComment},
    {SCE_RUST_COMMENTBLOCKDOC,      sComment},
    {SCE_RUST_COMMENTLINEDOC,       sComment},
    {SCE_RUST_NUMBER,               sNumberLiteral},
    {SCE_RUST_WORD,                 sKeyword1},
    {SCE_RUST_WORD2,                sKeyword2},
    {SCE_RUST_STRING,               sStringLiteral},
    {SCE_RUST_STRINGR,              sStringLiteral},
    {SCE_RUST_CHARACTER,            sCharLiteral},
    {SCE_RUST_MACRO,                sPreprocessor},
    {SCE_RUST_OPERATOR,             sOperator},
    {SCE_RUST_LIFETIME,             sGlobals},
    {SCE_RUST_BYTESTRING,           sEscapeSequence},
    {SCE_RUST_BYTESTRINGR,          sEscapeSequence},
    {SCE_RUST_BYTECHARACTER,        sEscapeSequence},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, const char *> keywordsRust[] = {
    {0,
"as break const continue crate dyn else enum extern false fn for if impl in let "
"loop match mod move mut pub ref return self Self static struct super trait true "
"type unsafe use where while "
    },
    {1,
"bool u8 u16 u32 u64 u128 i8 i16 i32 i64 i128 f32 f64 usize isize char str Pair "
"Box String List"
    },
    {(uchar) -1, nullptr},
};

static constexpr pair<uchar, Styles> stylesPython[] = {
    {SCE_P_DEFAULT,                 sNormal},
    {SCE_P_COMMENTLINE,             sComment},
    {SCE_P_NUMBER,                  sNumberLiteral},
    {SCE_P_STRING,                  sStringLiteral},
    {SCE_P_CHARACTER,               sCharLiteral},
    {SCE_P_WORD,                    sKeyword1},
    {SCE_P_TRIPLE,                  sStringLiteral},
    {SCE_P_TRIPLEDOUBLE,            sStringLiteral},
    {SCE_P_CLASSNAME,               sNormal},
    {SCE_P_DEFNAME,                 sNormal},
    {SCE_P_OPERATOR,                sOperator},
    {SCE_P_IDENTIFIER,              sNormal},
    {SCE_P_COMMENTBLOCK,            sComment},
    {SCE_P_STRINGEOL,               sNormal},
    {SCE_P_WORD2,                   sGlobals},
    {SCE_P_DECORATOR,               sPreprocessor},
    {SCE_P_FSTRING,                 sStringLiteral},
    {SCE_P_FCHARACTER,              sCharLiteral},
    {SCE_P_FTRIPLE,                 sStringLiteral},
    {SCE_P_FTRIPLEDOUBLE,           sStringLiteral},
    {(uchar) -1, {}},
};

static constexpr pair<uchar, const char *> keywordsPython[] = {
    {0,
"and as assert break class continue def del elif else except exec finally for "
"from global if import in is lambda not or pass print raise return try while "
"with yield"
    },
    {1,
"int float complex list tuple range str bytes bytearray memoryview set frozenset "
"dict "
    },
    {(uchar) -1, nullptr},
};

static constexpr pair<const char *, const char *> propertiesPython[] = {
    {"lexer.python.keywords2.no.sub.identifiers",       "1"},
    {nullptr, nullptr},
};

struct LexerInfo {
    const pair<uchar, Styles> *styles {nullptr};
    const pair<uchar, const char *> *keywords {nullptr};
    const pair<const char *, const char *> *properties {nullptr};
};

static const const_unordered_map<uchar, LexerInfo> lexerStyles = {
    {SCLEX_CPP, {stylesC, keywordsC, propertiesC}},
    {SCLEX_MAKEFILE, {stylesMake, nullptr, nullptr}},
    {SCLEX_ASM, {stylesAsm, nullptr, nullptr}},
    {SCLEX_COFFEESCRIPT, {stylesCoffeeScript, nullptr, nullptr}},
    {SCLEX_RUST, {stylesRust, keywordsRust, nullptr}},
    {SCLEX_PYTHON, {stylesPython, keywordsPython, propertiesPython}},
};

void loadLexer(int lexerId, EditorWindow &win)
{
    auto &editor = win.editor;
    editor.WndProc(SCI_SETLEXER, lexerId, 0U);

    auto [styles, keywords, properties] = lexerStyles[lexerId];
    if (styles) {
        while (styles->first != (uchar) -1) {
            editor.setStyleColor(styles->first, styleDefaults[styles->second]);
            ++styles;
        }
    }
    if (keywords) {
        while (keywords->first != (uchar) -1) {
            editor.WndProc(SCI_SETKEYWORDS, keywords->first, (sptr_t) keywords->second);
            ++keywords;
        }
    }
    if (properties) {
        while (properties->first) {
            editor.WndProc(SCI_SETPROPERTY, (sptr_t) properties->first, (sptr_t) properties->second);
            ++properties;
        }
    }

    editor.WndProc(SCI_COLOURISE, 0, -1);
    win.redrawEditor();
}

void setUpStyles(EditorWindow &win)
{
    auto &editor = win.editor;
    win.editorView.setFillColor(styleDefaults[sNormal]);
    editor.setStyleColor(STYLE_DEFAULT, styleDefaults[sNormal]);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    editor.setSelectionColor(styleDefaults[sSelection]);
    editor.setWhitespaceColor(styleDefaults[sWhitespace]);
    editor.setStyleColor(STYLE_CONTROLCHAR, styleDefaults[sCtrlChar]);
    editor.setStyleColor(STYLE_LINENUMBER, styleDefaults[sLineNums]);
}
