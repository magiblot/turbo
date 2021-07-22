#define Uses_MsgBox
#include <tvision/tv.h>

#include <fmt/core.h>
#include <turbo/editstates.h>
#include <turbo/tscintilla.h>

namespace turbo {

/////////////////////////////////////////////////////////////////////////
// LineNumbersWidth

int LineNumbersWidth::update(Scintilla::TScintillaEditor &editor)
{
    int newWidth = enabled ? calcWidth(editor) : 0;
    editor.WndProc(SCI_SETMARGINWIDTHN, 0, newWidth); // Does nothing if width hasn't changed.
    return newWidth;
}

int LineNumbersWidth::calcWidth(Scintilla::TScintillaEditor &editor)
{
    int width = 1;
    size_t lines = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
    while (lines /= 10)
        ++width;
    if (width < minWidth)
        width = minWidth;
    return width;
}

/////////////////////////////////////////////////////////////////////////
// WrapState

bool WrapState::toggle(Scintilla::TScintillaEditor &editor, bool dialog)
{
    bool proceed = true;
    if (enabled)
    {
        auto line = editor.getFirstVisibleDocumentLine();
        editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        editor.WndProc(SCI_SETFIRSTVISIBLELINE, line, 0U);
        enabled = false;
    }
    else
    {
        const int size = editor.WndProc(SCI_GETLENGTH, 0U, 0U);
        const bool documentBig = size >= (1 << 19);
        if (documentBig && !confirmedOnce)
        {
            if (dialog)
            {
                const int width = editor.WndProc(SCI_GETSCROLLWIDTH, 0U, 0U);
                auto &&text = fmt::format("This document is very big and the longest of its lines is at least {} characters long.\nAre you sure you want to enable line wrapping?", width);
                ushort res = messageBox(text, mfInformation | mfYesButton | mfNoButton);
                proceed = confirmedOnce = (res == cmYes);
            }
            else
                proceed = false;
        }
        if (proceed)
        {
            editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
            enabled = true;
        }
    }
    return proceed;
}

/////////////////////////////////////////////////////////////////////////
// AutoIndent

void AutoIndent::applyToCurrentLine(Scintilla::TScintillaEditor &editor)
{
    if (enabled)
    {
        auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
        auto line = editor.WndProc(SCI_LINEFROMPOSITION, pos, 0U);
        if (line > 0)
        {
            auto indentation = editor.WndProc(SCI_GETLINEINDENTATION, line - 1, 0U);
            if (indentation > 0)
            {
                editor.WndProc(SCI_SETLINEINDENTATION, line, indentation);
                editor.WndProc(SCI_VCHOME, 0U, 0U);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////

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

void ensureNewlineAtEnd(Scintilla::TScintillaEditor &editor)
{
    int EOLType = editor.WndProc(SCI_GETEOLMODE, 0U, 0U);
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

} // namespace turbo
