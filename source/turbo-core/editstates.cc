#define Uses_MsgBox
#include <tvision/tv.h>

#include <fmt/core.h>
#include <turbo/editstates.h>
#include <turbo/scintilla.h>

namespace turbo {

/////////////////////////////////////////////////////////////////////////
// LineNumbersWidth

int LineNumbersWidth::update(Scintilla &scintilla)
{
    int newWidth = enabled ? calcWidth(scintilla) : 0;
    call(scintilla, SCI_SETMARGINWIDTHN, 0, newWidth); // Does nothing if width hasn't changed.
    return newWidth;
}

int LineNumbersWidth::calcWidth(Scintilla &scintilla)
{
    int width = 1;
    size_t lines = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    while (lines /= 10)
        ++width;
    if (width < minWidth)
        width = minWidth;
    return width;
}

/////////////////////////////////////////////////////////////////////////
// WrapState

bool WrapState::toggle(Scintilla &scintilla, TFuncView<bool(int)> wrapIfBig)
{
    bool proceed = true;
    if (enabled)
    {
        auto line = call(scintilla, SCI_GETFIRSTVISIBLELINE, 0U, 0U);
        call(scintilla, SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        call(scintilla, SCI_SETFIRSTVISIBLELINE, line, 0U);
        enabled = false;
    }
    else
    {
        int size = call(scintilla, SCI_GETLENGTH, 0U, 0U);
        bool documentBig = size >= (1 << 19);
        if (documentBig && !confirmedOnce)
        {
            const int width = call(scintilla, SCI_GETSCROLLWIDTH, 0U, 0U);
            proceed = confirmedOnce = wrapIfBig(width);
        }
        if (proceed)
        {
            call(scintilla, SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
            enabled = true;
        }
    }
    return proceed;
}

bool defWrapIfBig(int width)
{
    auto &&text = fmt::format("This document is very big and the longest of its lines is at least {} characters long.\nAre you sure you want to enable line wrapping?", width);
    return messageBox(text, mfInformation | mfYesButton | mfNoButton) == cmYes;
}

/////////////////////////////////////////////////////////////////////////
// AutoIndent

void AutoIndent::applyToCurrentLine(Scintilla &scintilla)
{
    if (enabled)
    {
        auto pos = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
        auto line = call(scintilla, SCI_LINEFROMPOSITION, pos, 0U);
        if (line > 0)
        {
            auto indentation = call(scintilla, SCI_GETLINEINDENTATION, line - 1, 0U);
            if (indentation > 0)
            {
                call(scintilla, SCI_SETLINEINDENTATION, line, indentation);
                call(scintilla, SCI_VCHOME, 0U, 0U);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////

void stripTrailingSpaces(Scintilla &scintilla)
{
    Sci::Line lineCount = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    for (Sci::Line line = 0; line < lineCount; ++line) {
        Sci::Position lineStart = call(scintilla, SCI_POSITIONFROMLINE, line, 0U);
        Sci::Position lineEnd = call(scintilla, SCI_GETLINEENDPOSITION, line, 0U);
        Sci::Position i;
        for (i = lineEnd - 1; i >= lineStart; --i) {
            char ch = call(scintilla, SCI_GETCHARAT, i, 0U);
            if (ch != ' ' && ch != '\t')
                break;
        }
        if (i != lineEnd - 1) { // Not first iteration, trailing whitespace.
            call(scintilla, SCI_SETTARGETRANGE, i + 1, lineEnd);
            call(scintilla, SCI_REPLACETARGET, 0, (sptr_t) "");
        }
    }
}

void ensureNewlineAtEnd(Scintilla &scintilla)
{
    int EOLType = call(scintilla, SCI_GETEOLMODE, 0U, 0U);
    Sci::Line lineCount = call(scintilla, SCI_GETLINECOUNT, 0U, 0U);
    Sci::Position docEnd = call(scintilla, SCI_POSITIONFROMLINE, lineCount, 0U);
    if ( lineCount == 1 || (lineCount > 1 &&
         docEnd > call(scintilla, SCI_POSITIONFROMLINE, lineCount - 1, 0U)) )
    {
        std::string_view EOL = (EOLType == SC_EOL_CRLF) ? "\r\n" :
                               (EOLType == SC_EOL_CR)   ? "\r"   :
                                                          "\n";
        call(scintilla, SCI_APPENDTEXT, EOL.size(), (sptr_t) EOL.data());
    }
}

} // namespace turbo
