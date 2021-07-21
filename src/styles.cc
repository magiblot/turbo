#include <tvision/tv.h>
#include "util.h"
#include "editwindow.h"
#include "tscintilla.h"
#include "styles.h"
#include "app.h"
#include <utility>
using namespace Scintilla;
using std::pair;

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

static const const_unordered_map<std::string_view, Language> mime2lang = {
    {"text/x-c++",                  langCPP},
    {"text/x-c",                    langCPP},
    {"text/x-script.python",        langPython},
    {"application/json",            langJSON},
    {"text/x-shellscript",          langBash},
    {"text/x-makefile",             langMakefile},
    {"text/x-diff",                 langDiff},
};

static const const_unordered_map<std::string_view, Language> ext2lang = {
    {".js",         langJavaScript},
    {".jsx",        langJavaScript},
    {".mjs",        langJavaScript},
    {".asm",        langAsm},
    {".s",          langAsm},
    {".S",          langAsm},
    {".c",          langCPP},
    {".cc",         langCPP},
    {".cpp",        langCPP},
    {".cxx",        langCPP},
    {".h",          langCPP},
    {".hh",         langCPP},
    {".hpp",        langCPP},
    {".hxx",        langCPP},
    {".py",         langPython},
    {".htm",        langHTML},
    {".html",       langHTML},
    {".mhtml",      langHTML},
    {".xml",        langXML},
    {".vb",         langVB},
    {".pl",         langPerl},
    {".pm",         langPerl},
    {".bat",        langBatch},
    {".tex",        langLaTex},
    {".lua",        langLua},
    {".diff",       langDiff},
    {".ads",        langAda},
    {".adb",        langAda},
    {".lsp",        langLisp},
    {".rb",         langRuby},
    {".tcl",        langTcl},
    {".vbs",        langVBScript},
    {".m",          langMATLAB},
    {".css",        langCSS},
    {".yaml",       langYAML},
    {".erl",        langErlang},
    {".hrl",        langErlang},
    {".st",         langSmalltalk},
    {".md",         langMarkdown},
    {".rs",         langRust},
    {".java",       langCPP},
    {"Makefile",    langMakefile},
    {"PKGBUILD",    langBash},
};

bool ThemingState::detectLanguage(const char *filePath, Scintilla::TScintillaEditor &editor)
{
    Language lang = langNone;
    {
        auto ext = TPath::extname(filePath);
        if (!ext.empty())
            lang = ext2lang[ext];
        else
        {
            auto name = TPath::basename(filePath);
            lang = ext2lang[name];
        }
    }
#ifdef HAVE_MAGIC
    if (lang == langNone) {
        magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
        if (magic_cookie) {
            if (magic_load(magic_cookie, nullptr) == 0)
            {
                const char *mimeType = magic_file(magic_cookie, filePath);
                if (mimeType)
                    lang = mime2lang[mimeType];
            }
            if ( magic_setflags(magic_cookie, MAGIC_MIME_ENCODING) == 0 &&
                 magic_load(magic_cookie, nullptr) == 0 )
            {
                const char *mimeEncoding = magic_file(magic_cookie, filePath);
                if (mimeEncoding)
                    ;
            }
        }
        magic_close(magic_cookie);
    }
#endif

    if (loadLexer(lang, editor))
    {
        language = lang;
        return true;
    }
    return false;
}

static constexpr TColorAttr schemaDefault[StyleCount] = {
    /* sNormal           */ {{}      , {}                       },
    /* sSelection        */ {'\x1'   , '\x7'                    },
    /* sWhitespace       */ {'\x5'   , {}                       },
    /* sCtrlChar         */ {'\xD'   , {}                       },
    /* sLineNums         */ {'\x6'   , {}                       },
    /* sKeyword1         */ {'\xE'   , {}                       },
    /* sKeyword2         */ {'\xA'   , {}                       },
    /* sMisc             */ {'\x9'   , {}                       },
    /* sPreprocessor     */ {'\x2'   , {}                       },
    /* sOperator         */ {'\xD'   , {}                       },
    /* sComment          */ {'\x6'   , {}                       },
    /* sStringLiteral    */ {'\xC'   , {}                       },
    /* sCharLiteral      */ {'\xC'   , {}                       },
    /* sNumberLiteral    */ {'\x3'   , {}                       },
    /* sEscapeSequence   */ {'\xB'   , {}                       },
    /* sError            */ {'\x0'   , '\x3'                    },
    /* sBraceMatch       */ {'\xE'   , {}      , slBold         },
    /* sFramePassive     */ '\x07',
    /* sFrameActive      */ '\x0F',
    /* sFrameIcon        */ '\x0A',
    /* sStaticText       */ '\x0F',
    /* sLabelNormal      */ '\x08',
    /* sLabelSelected    */ '\x0F',
    /* sLabelShortcut    */ '\x06',
    /* sButtonNormal     */ '\x20',
    /* sButtonDefault    */ '\x2B',
    /* sButtonSelected   */ '\x2F',
    /* sButtonDisabled   */ '\x78',
    /* sButtonShortcut   */ '\x2E',
    /* sButtonShadow     */ '\x08',
};


ThemingState::ThemingState() noexcept :
    language(langNone),
    lexInfo(nullptr),
    schema(schemaDefault)
{
    if (TurboApp::app)
    {
        TPalette &pal = TurboApp::app->getPalette();
        pal[edFramePassive  ] = normalize(sFramePassive  );
        pal[edFrameActive   ] = normalize(sFrameActive   );
        pal[edFrameIcon     ] = normalize(sFrameIcon     );
        pal[edStaticText    ] = normalize(sStaticText    );
        pal[edLabelNormal   ] = normalize(sLabelNormal   );
        pal[edLabelSelected ] = normalize(sLabelSelected );
        pal[edLabelShortcut ] = normalize(sLabelShortcut );
        pal[edButtonNormal  ] = normalize(sButtonNormal  );
        pal[edButtonDefault ] = normalize(sButtonDefault );
        pal[edButtonSelected] = normalize(sButtonSelected);
        pal[edButtonDisabled] = normalize(sButtonDisabled);
        pal[edButtonShortcut] = normalize(sButtonShortcut);
        pal[edButtonShadow  ] = normalize(sButtonShadow  );
    }
}

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
    {SCE_C_GLOBALCLASS,             sMisc},
    {SCE_C_PREPROCESSORCOMMENT,     sComment},
    {SCE_C_PREPROCESSORCOMMENTDOC,  sComment},
    {SCE_C_ESCAPESEQUENCE,          sEscapeSequence},
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
};

static constexpr pair<const char *, const char *> propertiesC[] = {
    {"styling.within.preprocessor",         "1"},
    {"lexer.cpp.track.preprocessor",        "0"},
    {"lexer.cpp.escape.sequence",           "1"},
};

static constexpr TSpan<const char> bracesC = "[](){}";

static constexpr pair<uchar, Styles> stylesMake[] = {
    {SCE_MAKE_DEFAULT,              sNormal},
    {SCE_MAKE_COMMENT,              sComment},
    {SCE_MAKE_TARGET,               sKeyword1},
    {SCE_MAKE_IDENTIFIER,           sPreprocessor},
    {SCE_MAKE_PREPROCESSOR,         sPreprocessor},
    {SCE_MAKE_OPERATOR,             sOperator},
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
};

static constexpr pair<uchar, const char *> keywordsJavaScript[] = {
    {0,
"await break case catch continue default do else export false finally "
"for get if import new null return set super switch this throw true try while "
"with yield"
    },
    {1,
"async class const debugger delete enum eval extends function in instanceof let "
"static typeof var void"
    },
    {3,
"arguments Array ArrayBuffer AsyncFunction Atomics BigInt BigInt64Array "
"BigUint64Array Boolean DataView Date Error EvalError Float32Array Float64Array "
"Function Generator GeneratorFunction globalThis Infinity Int8Array Int16Array "
"Int32Array InternalError Intl JSON Map Math NaN Number Object Promise Proxy "
"RangeError ReferenceError Reflect RegExp Set String SyntaxError TypeError URIError "
"SharedArrayBuffer Symbol Uint8Array Uint8ClampedArray Uint16Array Uint32Array "
"undefined WeakMap WeakSet WebAssembly"
    },
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
    {SCE_RUST_LIFETIME,             sMisc},
    {SCE_RUST_BYTESTRING,           sEscapeSequence},
    {SCE_RUST_BYTESTRINGR,          sEscapeSequence},
    {SCE_RUST_BYTECHARACTER,        sEscapeSequence},
};

static constexpr pair<uchar, const char *> keywordsRust[] = {
    {0,
"as break const continue crate dyn else enum extern false fn for if impl in let "
"loop match mod move mut pub ref return self Self static struct super trait true "
"type unsafe use where while "
    },
    {1,
"bool u8 u16 u32 u64 u128 i8 i16 i32 i64 i128 f32 f64 usize isize char str Pair "
"Box box String List"
    },
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
    {SCE_P_WORD2,                   sMisc},
    {SCE_P_DECORATOR,               sPreprocessor},
    {SCE_P_FSTRING,                 sStringLiteral},
    {SCE_P_FCHARACTER,              sCharLiteral},
    {SCE_P_FTRIPLE,                 sStringLiteral},
    {SCE_P_FTRIPLEDOUBLE,           sStringLiteral},
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
};

static constexpr pair<const char *, const char *> propertiesPython[] = {
    {"lexer.python.keywords2.no.sub.identifiers",       "1"},
};

static constexpr pair<uchar, Styles> stylesBash[] = {
    {SCE_SH_DEFAULT,                sNormal},
    {SCE_SH_ERROR,                  sError},
    {SCE_SH_COMMENTLINE,            sComment},
    {SCE_SH_NUMBER,                 sNumberLiteral},
    {SCE_SH_WORD,                   sKeyword2},
    {SCE_SH_STRING,                 sStringLiteral},
    {SCE_SH_CHARACTER,              sCharLiteral},
    {SCE_SH_OPERATOR,               sOperator},
    {SCE_SH_IDENTIFIER,             sNormal},
    {SCE_SH_SCALAR,                 sKeyword1},
    {SCE_SH_PARAM,                  sKeyword1},
    {SCE_SH_BACKTICKS,              sKeyword1},
    {SCE_SH_HERE_DELIM,             sMisc},
    {SCE_SH_HERE_Q,                 sMisc},
};

static constexpr pair<uchar, const char *> keywordsBash[] = {
    {0,
// Keywords
"case do done elif else esac fi for function if in select then time until while "
// Builtins
"alias bg bind break builtin caller cd command compgen complete compopt continue "
"declare dirs disown echo enable eval exec exit export fc fg getopts hash help "
"history jobs kill let local logout mapfile popd printf pushd pwd read readarray "
"readonly return set shift shopt source suspend test times trap type typeset ulimit "
"umask unalias unset wait "
    },
};

static constexpr pair<uchar, Styles> stylesRuby[] = {
    {SCE_RB_DEFAULT,                sNormal},
    {SCE_RB_ERROR,                  sError},
    {SCE_RB_COMMENTLINE,            sComment},
    {SCE_RB_POD,                    sMisc},
    {SCE_RB_NUMBER,                 sNumberLiteral},
    {SCE_RB_WORD,                   sKeyword1},
    {SCE_RB_STRING,                 sStringLiteral},
    {SCE_RB_CHARACTER,              sCharLiteral},
    {SCE_RB_CLASSNAME,              sNormal},
    {SCE_RB_DEFNAME,                sNormal},
    {SCE_RB_OPERATOR,               sOperator},
    {SCE_RB_IDENTIFIER,             sNormal},
    {SCE_RB_REGEX,                  sNormal},
    {SCE_RB_GLOBAL,                 sNormal},
    {SCE_RB_SYMBOL,                 sKeyword2},
    {SCE_RB_MODULE_NAME,            sNormal},
    {SCE_RB_INSTANCE_VAR,           sNormal},
    {SCE_RB_CLASS_VAR,              sNormal},
    {SCE_RB_BACKTICKS,              sKeyword1},
    {SCE_RB_HERE_DELIM,             sMisc},
    {SCE_RB_HERE_Q,                 sMisc},
    {SCE_RB_STRING_QQ,              sMisc},
    {SCE_RB_STRING_QX,              sMisc},
    {SCE_RB_STRING_QR,              sMisc},
    {SCE_RB_STRING_QW,              sMisc},
    {SCE_RB_WORD_DEMOTED,           sNormal},
    {SCE_RB_STDIN,                  sMisc},
    {SCE_RB_STDOUT,                 sMisc},
    {SCE_RB_STDERR,                 sMisc},
    {SCE_RB_UPPER_BOUND,            sMisc},
};

static constexpr pair<uchar, const char *> keywordsRuby[] = {
    {0,
"__ENCODING__ __LINE__ __FILE__ BEGIN END "
"alias and begin break case class def defined? do else elsif end ensure false "
"for if in module next nil not or redo rescue retry return self super then true "
"undef unless until when while yield "
"public require require_relative "
    },
};

struct LexerInfo {
    const int lexer {SCLEX_NULL};
    const LexerStyles styles;
    const LexerKeywords keywords;
    const LexerProperties properties;
    const TSpan<const char> braces;
};

static const std::unordered_map<Language, LexerInfo> lexerStyles = {
    {langCPP, {SCLEX_CPP, stylesC, keywordsC, propertiesC, bracesC}},
    {langMakefile, {SCLEX_MAKEFILE, stylesMake, nullptr, nullptr, bracesC}},
    {langAsm, {SCLEX_ASM, stylesAsm, nullptr, nullptr, bracesC}},
    {langJavaScript, {SCLEX_CPP, stylesC, keywordsJavaScript, propertiesC, bracesC}},
    {langRust, {SCLEX_RUST, stylesRust, keywordsRust, nullptr, bracesC}},
    {langPython, {SCLEX_PYTHON, stylesPython, keywordsPython, propertiesPython, bracesC}},
    {langBash, {SCLEX_BASH, stylesBash, keywordsBash, nullptr, bracesC}},
    {langRuby, {SCLEX_RUBY, stylesRuby, keywordsRuby, nullptr, bracesC}},
};

bool ThemingState::loadLexer(Language lang, Scintilla::TScintillaEditor &editor)
{
    auto it = lexerStyles.find(lang);
    if (it != lexerStyles.end())
    {
        lexInfo = &it->second;
        editor.WndProc(SCI_SETLEXER, lexInfo->lexer, 0U);
        for (const auto &style : lexInfo->styles)
            editor.setStyleColor(style.first, normalize(style.second));
        for (const auto &keyword : lexInfo->keywords)
            editor.WndProc(SCI_SETKEYWORDS, keyword.first, (sptr_t) keyword.second);
        for (const auto &property : lexInfo->properties)
            editor.WndProc(SCI_SETPROPERTY, (sptr_t) property.first, (sptr_t) property.second);
        editor.WndProc(SCI_COLOURISE, 0, -1);
    }
    else
        lexInfo = nullptr;
    return lexInfo;
}

static TColorAttr merge(const TColorAttr &from, const TColorAttr &into)
{
    auto f_fg = ::getFore(from),
         f_bg = ::getBack(from),
         i_fg = ::getFore(into),
         i_bg = ::getBack(into);
    return {
        f_fg.isDefault() ? i_fg : f_fg,
        f_bg.isDefault() ? i_bg : f_bg,
        ::getStyle(from),
    };
}

TColorAttr ThemingState::normalize(Styles index) const
{
    auto normal = schema[sNormal];
    if (index != sNormal)
        return merge(schema[index], normal);
    return normal;
}

void ThemingState::resetStyles(Scintilla::TScintillaEditor &editor) const
{
    editor.setStyleColor(STYLE_DEFAULT, schema[sNormal]);
    editor.WndProc(SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    editor.setSelectionColor(schema[sSelection]);
    editor.setWhitespaceColor(schema[sWhitespace]);
    editor.setStyleColor(STYLE_CONTROLCHAR, normalize(sCtrlChar));
    editor.setStyleColor(STYLE_LINENUMBER, normalize(sLineNums));
}

TColorAttr ThemingState::braceAttr(LexerStyles styles, uchar sciStyle) const
{
    for (const auto &lexStyle : styles)
        if (lexStyle.first == sciStyle)
            return merge(schema[sBraceMatch], normalize(lexStyle.second));
    return schema[sError];
}

static bool isBrace(TSpan<const char> braces, char ch)
{
    return memchr(braces.data(), ch, braces.size()) != nullptr;
}

void ThemingState::updateBraces(Scintilla::TScintillaEditor &editor) const
{
    if (lexInfo)
    {
        auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
        auto ch = editor.WndProc(SCI_GETCHARAT, pos, 0U);
        bool braceFound = false;
        if (isBrace(lexInfo->braces, ch))
        {
            // We must lex any newly inserted text so that it has the right style.
            editor.idleWork();
            // Scintilla already makes sure that both braces have the same style.
            auto matchPos = editor.WndProc(SCI_BRACEMATCH, pos, 0U);
            if (matchPos != -1)
            {
                uchar sciStyle = editor.WndProc(SCI_GETSTYLEAT, pos, 0U);
                auto attr = braceAttr(lexInfo->styles, sciStyle);
                editor.setStyleColor(STYLE_BRACELIGHT, attr);
                editor.WndProc(SCI_BRACEHIGHLIGHT, pos, matchPos);
                braceFound = true;
            }
        }
        if (!braceFound)
            editor.WndProc(SCI_BRACEHIGHLIGHT, -1, -1);
    }
}
