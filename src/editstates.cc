#include "editstates.h"
#include "editwindow.h"

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

void ensureNewlineAtEnd(Scintilla::TScintillaEditor &editor, int EOLType)
{
    Sci::Line lineCount = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
    Sci::Position docEnd = editor.WndProc(SCI_POSITIONFROMLINE, lineCount, 0U);
    if ( lineCount == 1 || (lineCount > 1 &&
         docEnd > editor.WndProc(SCI_POSITIONFROMLINE, lineCount - 1, 0U)) )
    {
        std::string_view EOL = (EOLType == SC_EOL_CRLF) ? "\r\n" :
                               (EOLType == SC_EOL_CR)   ? "\r"   :
                                                          "\n";
        editor.WndProc(SCI_APPENDTEXT, EOL.size(), (sptr_t) EOL.data());
    }
}
