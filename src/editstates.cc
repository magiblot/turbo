#include "editstates.h"
#include "editwindow.h"
#include "styles.h"

/////////////////////////////////////////////////////////////////////////
// File type detection

#include "util.h"
#include <magic.h>

static const const_unordered_map<std::string_view, int> mime2lex = {
    {"text/x-c++",                  SCLEX_CPP},
    {"text/x-c",                    SCLEX_CPP},
    {"text/x-script.python",        SCLEX_PYTHON},
    {"application/json",            SCLEX_JSON},
    {"text/x-shellscript",          SCLEX_BASH},
    {"text/x-makefile",             SCLEX_MAKEFILE},
    {"text/x-diff",                 SCLEX_DIFF},
};

static const const_unordered_map<std::string_view, int> ext2lex = {
    {".js",         SCLEX_COFFEESCRIPT},
    {".jsx",        SCLEX_COFFEESCRIPT},
    {".mjs",        SCLEX_COFFEESCRIPT},
    {".asm",        SCLEX_ASM},
    {".s",          SCLEX_ASM},
    {".S",          SCLEX_ASM},
    {".c",          SCLEX_CPP},
    {".cc",         SCLEX_CPP},
    {".cpp",        SCLEX_CPP},
    {".cxx",        SCLEX_CPP},
    {".h",          SCLEX_CPP},
    {".hh",         SCLEX_CPP},
    {".hpp",        SCLEX_CPP},
    {".hxx",        SCLEX_CPP},
    {".py",         SCLEX_PYTHON},
    {".htm",        SCLEX_HTML},
    {".html",       SCLEX_HTML},
    {".mhtml",      SCLEX_HTML},
    {".xml",        SCLEX_XML},
    {".vb",         SCLEX_VB},
    {".pl",         SCLEX_PERL},
    {".pm",         SCLEX_PERL},
    {".bat",        SCLEX_BATCH},
    {".tex",        SCLEX_LATEX},
    {".lua",        SCLEX_LUA},
    {".diff",       SCLEX_DIFF},
    {".ads",        SCLEX_ADA},
    {".adb",        SCLEX_ADA},
    {".lsp",        SCLEX_LISP},
    {".rb",         SCLEX_RUBY},
    {".tcl",        SCLEX_TCL},
    {".vbs",        SCLEX_VBSCRIPT},
    {".m",          SCLEX_MATLAB},
    {".css",        SCLEX_CSS},
    {".yaml",       SCLEX_YAML},
    {".erl",        SCLEX_ERLANG},
    {".hrl",        SCLEX_ERLANG},
    {".st",         SCLEX_SMALLTALK},
    {".md",         SCLEX_MARKDOWN},
    {".rs",         SCLEX_RUST},
    {".java",       SCLEX_CPP},
};

void FileType::detect(EditorWindow &win)
{
    auto &file = win.file;
    int lexer = 0;
    int encoding = 0;
#ifdef HAVE_MAGIC
    {
        magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
        if (magic_cookie) {
            if (magic_load(magic_cookie, nullptr) == 0)
            {
                const char *mimeType = magic_file(magic_cookie, file.c_str());
                if (mimeType)
                    lexer = mime2lex[mimeType];
            }
            if ( magic_setflags(magic_cookie, MAGIC_MIME_ENCODING) == 0 &&
                 magic_load(magic_cookie, nullptr) == 0 )
            {
                const char *mimeEncoding = magic_file(magic_cookie, file.c_str());
                if (mimeEncoding)
                    ;
            }
        }
        magic_close(magic_cookie);
    }
#endif
    if (!lexer) {
        auto &&ext = file.extension();
        lexer = ext2lex[ext.native()];
    }

    if (!lexer)
        lexer = SCLEX_NULL;
    else
        win.lineNumbers.setState(true);

    loadLexer(lexer, win);
}
