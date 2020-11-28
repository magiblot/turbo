#include "editstates.h"
#include "editwindow.h"
#include "styles.h"

/////////////////////////////////////////////////////////////////////////
// File type detection

#include "util.h"

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
};

void FileType::detect(EditorWindow &win)
{
    auto &file = win.file;
    Language lang = langNone;
    [[maybe_unused]] int encoding = 0;
    {
        auto ext = TPath::extname(file);
        lang = ext2lang[ext];
    }
#ifdef HAVE_MAGIC
    if (!lang) {
        magic_t magic_cookie = magic_open(MAGIC_MIME_TYPE);
        if (magic_cookie) {
            if (magic_load(magic_cookie, nullptr) == 0)
            {
                const char *mimeType = magic_file(magic_cookie, file.c_str());
                if (mimeType)
                    lang = mime2lang[mimeType];
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
    if (lang != langNone)
        win.lineNumbers.setState(true);

    loadLexer(lang, win);
}

void stripTrailingSpaces(Scintilla::TScintillaEditor &editor)
{
    Sci::Line lineCount = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
    for (Sci::Line line = 0; line < lineCount; ++line) {
        Sci::Position lineStart = editor.WndProc(SCI_POSITIONFROMLINE, line, 0U);
        Sci::Position lineEnd = editor.WndProc(SCI_GETLINEENDPOSITION, line, 0U);
        Sci::Position i;
        for (i = lineEnd - 1; i >= lineStart; --i) {
            char ch = editor.WndProc(SCI_GETCHARAT, i, 0U);
            if (ch != ' ' && ch != '\t')
                break;
        }
        if (i != lineEnd - 1) { // Not first iteration, trailing whitespace.
            editor.WndProc(SCI_SETTARGETRANGE, i + 1, lineEnd);
            editor.WndProc(SCI_REPLACETARGET, 0, (sptr_t) "");
        }
    }
}
