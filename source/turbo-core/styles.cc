#include <tvision/tv.h>
#include <turbo/scintilla.h>
#include <turbo/styles.h>
#include <turbo/tpath.h>
#include <string_view>
#include "utils.h"

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

namespace turbo {

constexpr Language
    Language::CPP {"//", "/*", "*/"},
    Language::Makefile {"#"},
    Language::Asm {";"},
    Language::JavaScript {"//", "/*", "*/"},
    Language::Rust {"//", "/*", "*/"},
    Language::Python {"#"},
    Language::Bash {"#"},
    Language::Diff,
    Language::JSON {"//", "/*", "*/"},
    Language::HTML {{}, "<!--", "-->"},
    Language::XML {{}, "<!--", "-->"},
    Language::VB {"'"},
    Language::Perl {"#"},
    Language::Batch {"rem "},
    Language::LaTex {"%"},
    Language::Lua {"--", "--[[", "]]"},
    Language::Ada {"--"},
    Language::Lisp {";"},
    Language::Ruby {"#"},
    Language::Tcl {"#"},
    Language::VBScript {"#"},
    Language::MATLAB {"%"},
    Language::CSS {{}, "/*", "*/"},
    Language::YAML {"#"},
    Language::Erlang {"%"},
    Language::Smalltalk {{}, "\"", "\""},
    Language::Markdown;

static const const_unordered_map<std::string_view, const Language *> mime2lang = {
    {"text/x-c++",                  &Language::CPP},
    {"text/x-c",                    &Language::CPP},
    {"text/x-script.python",        &Language::Python},
    {"application/json",            &Language::JSON},
    {"text/x-shellscript",          &Language::Bash},
    {"text/x-makefile",             &Language::Makefile},
    {"text/x-diff",                 &Language::Diff},
    {"text/html",                   &Language::HTML},
};

static const const_unordered_map<std::string_view, const Language *> ext2lang = {
    {".js",                         &Language::JavaScript},
    {".jsx",                        &Language::JavaScript},
    {".mjs",                        &Language::JavaScript},
    {".asm",                        &Language::Asm},
    {".s",                          &Language::Asm},
    {".S",                          &Language::Asm},
    {".c",                          &Language::CPP},
    {".cc",                         &Language::CPP},
    {".cpp",                        &Language::CPP},
    {".cxx",                        &Language::CPP},
    {".h",                          &Language::CPP},
    {".hh",                         &Language::CPP},
    {".hpp",                        &Language::CPP},
    {".hxx",                        &Language::CPP},
    {".py",                         &Language::Python},
    {".htm",                        &Language::HTML},
    {".html",                       &Language::HTML},
    {".mhtml",                      &Language::HTML},
    {".xml",                        &Language::XML},
    {".vb",                         &Language::VB},
    {".pl",                         &Language::Perl},
    {".pm",                         &Language::Perl},
    {".bat",                        &Language::Batch},
    {".tex",                        &Language::LaTex},
    {".lua",                        &Language::Lua},
    {".diff",                       &Language::Diff},
    {".ads",                        &Language::Ada},
    {".adb",                        &Language::Ada},
    {".lsp",                        &Language::Lisp},
    {".rb",                         &Language::Ruby},
    {".tcl",                        &Language::Tcl},
    {".vbs",                        &Language::VBScript},
    {".m",                          &Language::MATLAB},
    {".css",                        &Language::CSS},
    {".erl",                        &Language::Erlang},
    {".hrl",                        &Language::Erlang},
    {".st",                         &Language::Smalltalk},
    {".md",                         &Language::Markdown},
    {".rs",                         &Language::Rust},
    {".java",                       &Language::CPP},
    {"Makefile",                    &Language::Makefile},
    {"PKGBUILD",                    &Language::Bash},
    {".json",                       &Language::JSON},
    {"eslintrc",                    &Language::JSON},
    {".jshintrc",                   &Language::JSON},
    {".jsonld",                     &Language::JSON},
    {".ipynb",                      &Language::JSON},
    {".babelrc",                    &Language::JSON},
    {".prettierrc",                 &Language::JSON},
    {".stylelintrc",                &Language::JSON},
    {".jsonc",                      &Language::JSON},
    {".jscop",                      &Language::JSON},
    {".yml",                        &Language::YAML},
    {".yaml",                       &Language::YAML},
    {".clang-format",               &Language::YAML},
    {".clang-tidy",                 &Language::YAML},
    {".mir",                        &Language::YAML},
    {".apinotes",                   &Language::YAML},
    {".ifs",                        &Language::YAML},
    {".sh",                         &Language::Bash},
};

const Language *detectFileLanguage(const char *filePath)
{
    const Language *lang = nullptr;
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
    if (!lang) {
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
    return lang;
}

extern constexpr ColorScheme schemeDefault =
{
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
    /* sReplaceHighlight */ {'\x0'   , '\xA'                    },
};

constexpr LexerSettings::StyleMapping stylesC[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsC[] =
{
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

constexpr LexerSettings::PropertyMapping propertiesC[] =
{
    {"styling.within.preprocessor",         "1"},
    {"lexer.cpp.track.preprocessor",        "0"},
    {"lexer.cpp.escape.sequence",           "1"},
};

constexpr LexerSettings::StyleMapping stylesMake[] =
{
    {SCE_MAKE_DEFAULT,              sNormal},
    {SCE_MAKE_COMMENT,              sComment},
    {SCE_MAKE_TARGET,               sKeyword1},
    {SCE_MAKE_IDENTIFIER,           sPreprocessor},
    {SCE_MAKE_PREPROCESSOR,         sPreprocessor},
    {SCE_MAKE_OPERATOR,             sOperator},
};

constexpr LexerSettings::StyleMapping stylesAsm[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsJavaScript[] =
{
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

constexpr LexerSettings::StyleMapping stylesHTML[] =
{
    // HTML
    { SCE_H_DEFAULT, sNormal},
    { SCE_H_TAG, sKeyword1},
    { SCE_H_TAGUNKNOWN, sError},
    { SCE_H_ATTRIBUTE, sKeyword2},
    { SCE_H_ATTRIBUTEUNKNOWN, sKeyword2},
    { SCE_H_NUMBER, sNumberLiteral},
    { SCE_H_DOUBLESTRING, sStringLiteral},
    { SCE_H_SINGLESTRING, sStringLiteral},
    { SCE_H_OTHER, sNormal},
    { SCE_H_COMMENT, sComment},
    { SCE_H_ENTITY, sNormal},
    { SCE_H_TAGEND, sKeyword1},
    { SCE_H_XMLSTART, sKeyword1},
    { SCE_H_XMLEND,  sKeyword1},
    { SCE_H_SCRIPT, sStringLiteral},
    { SCE_H_ASP, sStringLiteral},
    { SCE_H_ASPAT, sStringLiteral},
    { SCE_H_CDATA, sStringLiteral},
    { SCE_H_QUESTION, sNormal},
    { SCE_H_VALUE, sCharLiteral},
    { SCE_H_XCCOMMENT, sComment},
    // SGML (DTD)
    { SCE_H_SGML_DEFAULT, sNormal},
    { SCE_H_SGML_COMMAND, sKeyword2},
    { SCE_H_SGML_1ST_PARAM, sNormal},
    { SCE_H_SGML_DOUBLESTRING, sStringLiteral},
    { SCE_H_SGML_SIMPLESTRING, sStringLiteral},
    { SCE_H_SGML_ERROR, sError},
    { SCE_H_SGML_SPECIAL, sKeyword1},
    { SCE_H_SGML_ENTITY, sNormal},
    { SCE_H_SGML_COMMENT, sComment},
    { SCE_H_SGML_1ST_PARAM_COMMENT, sComment},
    { SCE_H_SGML_BLOCK_DEFAULT, sNormal},
    // JavaScript
    { SCE_HJ_DEFAULT, sNormal },
    { SCE_HJ_COMMENT, sComment },
    { SCE_HJ_COMMENTLINE, sComment },
    { SCE_HJ_COMMENTDOC, sComment },
    { SCE_HJ_NUMBER, sNumberLiteral },
    { SCE_HJ_WORD, sNormal },
    { SCE_HJ_KEYWORD, sKeyword1 },
    { SCE_HJ_DOUBLESTRING, sStringLiteral },
    { SCE_HJ_SINGLESTRING, sCharLiteral },
    { SCE_HJ_SYMBOLS, sCtrlChar },
    { SCE_HJ_REGEX, sMisc }
};

constexpr LexerSettings::KeywordMapping keywordsHTML[] =
{
    {0, // HTML
"a abbr address area article aside audio b base bdi bdo blockquote "
"body br button canvas caption cite code col colgroup data datalist dd del "
"details dfn dialog div dl dt em embed fieldset figure footer form h1 h2 h3 "
"h4 h5 h6 head header hgroup hr html i iframe img input ins kbd keygen label "
"legend li link main map mark menu menuitem meta meter nav noscript object "
"ol optgroup option output p param pre progress q rb rp rt rtc ruby s samp "
"script section select small source span strong style sub summary sup table "
"tbody td template textarea tfoot th thead time title tr track u ul var video "
"wbr strike tt acronym applet basefont big center dir font frame frameset noframes "
    // SVG
"svg animate animateMotion animateTransform circle clipPath defs desc discard "
"ellipse feBlend feColorMatrix feComponentTrasfer feComposite feConvolveMatrix "
"feDiffuseLighting feDisplacementMap feDistantLight feDropShadow feFlood feFuncA "
"feFuncB feFuncG feFuncR feGaussianBlur feImage feMerge feMergeNode feMorphology "
"feOffset fePointLight feSpecularLighting feSpotLight feTile feTurbulence filter "
"foreignObject g hatch hatchpath image line linearGradient marker mask metadata "
"mpath path pattern polygon polyline radialGradient rect set stop style switch "
"symbol text textPath title tspan use view "
    },
    {1, // JavaScript
"of await break case catch continue default do else export false finally for "
"get if import new null return set super switch this throw true try while with "
"yield async class const debugger delete enum eval extends function in instanceof "
"let static typeof var void undefined "
    },
    {2, // VBScript
" "
    },
    {3, // Python
" "
    },
    {4, // PHP
" "
    },
    {5, // SGML (DTD)
"DOCTYPE ELEMENT ATTLIST ENTITY PCDATA CDATA EMPTY SHORTREF USEMAP NOTATION IMPLIED "
"NDATA SYSTEM SGML HTML PUBLIC "
    },
};

constexpr LexerSettings::PropertyMapping propertiesHTML[] =
{
    {"asp.default.language",                "1"},
    {"html.tags.case.sensitive",            "0"},
    {"lexer.xml.allow.scripts",             "1"},
    {"lexer.html.mako",                     "0"},
    {"lexer.html.django",                   "0"},
    {"fold",                                "0"},
    {"fold.html",                           "0"},
    {"fold.html.preprocessor",              "0"},
    {"fold.compact",                        "0"},
    {"fold.hypertext.comment",              "0"},
    {"fold.hypertext.heredoc",              "0"},
    {"fold.xml.at.tag.open",                "0"},
};

constexpr LexerSettings::StyleMapping  stylesRust[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsRust[] =
{
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

constexpr LexerSettings::StyleMapping  stylesPython[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsPython[] =
{
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

constexpr LexerSettings::PropertyMapping propertiesPython[] =
{
    {"lexer.python.keywords2.no.sub.identifiers",       "1"},
};

constexpr LexerSettings::StyleMapping  stylesBash[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsBash[] =
{
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

constexpr LexerSettings::StyleMapping  stylesRuby[] =
{
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

constexpr LexerSettings::KeywordMapping keywordsRuby[] =
{
    {0,
"__ENCODING__ __LINE__ __FILE__ BEGIN END "
"alias and begin break case class def defined? do else elsif end ensure false "
"for if in module next nil not or redo rescue retry return self super then true "
"undef unless until when while yield "
"public require require_relative "
    },
};

constexpr LexerSettings::StyleMapping stylesJSON[] =
{
    {SCE_JSON_DEFAULT,              sNormal},
    {SCE_JSON_NUMBER,               sNumberLiteral},
    {SCE_JSON_STRING,               sStringLiteral},
    {SCE_JSON_STRINGEOL,            sStringLiteral},
    {SCE_JSON_PROPERTYNAME,         sPreprocessor},
    {SCE_JSON_ESCAPESEQUENCE,       sEscapeSequence},
    {SCE_JSON_LINECOMMENT,          sComment},
    {SCE_JSON_BLOCKCOMMENT,         sComment},
    {SCE_JSON_OPERATOR,             sOperator},
    {SCE_JSON_URI,                  sStringLiteral},
    {SCE_JSON_COMPACTIRI,           sKeyword2},
    {SCE_JSON_KEYWORD,              sKeyword1},
    {SCE_JSON_LDKEYWORD,            sKeyword2},
    {SCE_JSON_ERROR,                sError},
};

constexpr LexerSettings::KeywordMapping keywordsJSON[] =
{
    {0, "false true null"},
    {1,
"@id @context @type @value @language @container @list @set @reverse @index "
"@base @vocab @graph "
    },
};

constexpr LexerSettings::PropertyMapping propertiesJSON[] =
{
    {"lexer.json.escape.sequence", "1"},
    {"lexer.json.allow.comments", "1"},
};

constexpr LexerSettings::StyleMapping stylesYAML[] =
{
    {SCE_YAML_DEFAULT,              sNormal},
    {SCE_YAML_COMMENT,              sComment},
    {SCE_YAML_IDENTIFIER,           sPreprocessor},
    {SCE_YAML_KEYWORD,              sKeyword1},
    {SCE_YAML_NUMBER,               sNumberLiteral},
    {SCE_YAML_REFERENCE,            sKeyword2},
    {SCE_YAML_DOCUMENT,             sMisc},
    {SCE_YAML_TEXT,                 sStringLiteral},
    {SCE_YAML_ERROR,                sError},
    {SCE_YAML_OPERATOR,             sOperator},
};

constexpr LexerSettings::KeywordMapping keywordsYAML[] =
{
    {0, "true false yes no"},
};

constexpr struct { const Language *language; LexerSettings lexer; } builtInLexers[] =
{
    {&Language::CPP, {SCLEX_CPP, stylesC, keywordsC, propertiesC}},
    {&Language::Makefile, {SCLEX_MAKEFILE, stylesMake, nullptr, nullptr}},
    {&Language::Asm, {SCLEX_ASM, stylesAsm, nullptr, nullptr}},
    {&Language::JavaScript, {SCLEX_CPP, stylesC, keywordsJavaScript, propertiesC}},
    {&Language::Rust, {SCLEX_RUST, stylesRust, keywordsRust, nullptr}},
    {&Language::Python, {SCLEX_PYTHON, stylesPython, keywordsPython, propertiesPython}},
    {&Language::Bash, {SCLEX_BASH, stylesBash, keywordsBash, nullptr}},
    {&Language::Ruby, {SCLEX_RUBY, stylesRuby, keywordsRuby, nullptr}},
    {&Language::JSON, {SCLEX_JSON, stylesJSON, keywordsJSON, propertiesJSON}},
    {&Language::YAML, {SCLEX_YAML, stylesYAML, keywordsYAML, nullptr}},
    {&Language::HTML, {SCLEX_HTML, stylesHTML, keywordsHTML, propertiesHTML}},
};

TColorAttr coalesce(TColorAttr from, TColorAttr into)
{
    auto f_fg = ::getFore(from), f_bg = ::getBack(from),
         i_fg = ::getFore(into), i_bg = ::getBack(into);
    return {
        f_fg.isDefault() ? i_fg : f_fg,
        f_bg.isDefault() ? i_bg : f_bg,
        ::getStyle(from),
    };
}

const LexerSettings *findBuiltInLexer(const Language *language)
{
    for (const auto &l : builtInLexers)
        if (l.language == language)
            return &l.lexer;
    return nullptr;
}

} // namespace turbo
