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
    {"lexer.cpp.track.preprocessor",        "1"},
    {"lexer.cpp.escape.sequence",           "1"},
    {nullptr, nullptr},
};

struct LexerInfo {
    const pair<uchar, Styles> *styles {nullptr};
    const pair<uchar, const char *> *keywords {nullptr};
    const pair<const char *, const char *> *properties {nullptr};
};

static const const_unordered_map<uchar, LexerInfo> lexerStyles = {
    {SCLEX_CPP, {stylesC, keywordsC, propertiesC}},
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
