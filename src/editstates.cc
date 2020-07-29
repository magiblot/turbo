#include "editstates.h"
#include "editwindow.h"

/////////////////////////////////////////////////////////////////////////
// File type detection

#include "util.h"
#include <magic.h>

static const const_unordered_map<std::string_view, int> mime2lex = {
    {"text/x-c++",                  SCLEX_CPP},
    {"text/x-c",                    SCLEX_CPP},
    {"text/x-script.python",        SCLEX_PYTHON},
    {"application/json",            SCLEX_JSON},
    {"text/x-shellscript",          SCLEX_BASH}
};

static const const_unordered_map<std::string_view, int> ext2lex = {
    {".js",         SCLEX_COFFEESCRIPT},
    {".jsx",        SCLEX_COFFEESCRIPT},
    {".mjs",        SCLEX_COFFEESCRIPT},
    {".asm",        SCLEX_ASM},
    {".s",          SCLEX_ASM},
    {".S",          SCLEX_ASM}
};

void FileType::detect(EditorWindow &win)
{
    auto &file = win.file;
    auto &editor = win.editor;
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

    editor.WndProc(SCI_SETLEXER, lexer, 0U);
    editor.WndProc(SCI_COLOURISE, 0, -1);

}
