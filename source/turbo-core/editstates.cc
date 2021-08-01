#define Uses_MsgBox
#include <tvision/tv.h>

#include <fmt/core.h>
#include <turbo/editstates.h>
#include <turbo/scintilla.h>
#include <turbo/scintilla/internals.h>

namespace turbo {

/////////////////////////////////////////////////////////////////////////
// LineNumbersWidth

int LineNumbersWidth::update(TScintilla &scintilla)
{
    int newWidth = enabled ? calcWidth(scintilla) : 0;
    call(scintilla, SCI_SETMARGINWIDTHN, 0, newWidth); // Does nothing if width hasn't changed.
    return newWidth;
}

int LineNumbersWidth::calcWidth(TScintilla &scintilla)
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

void WrapState::setState(bool enable, TScintilla &scintilla, TFuncView<bool(int)> confirmWrap)
{
    if (!enable)
    {
        auto line = call(scintilla, SCI_GETFIRSTVISIBLELINE, 0U, 0U);
        call(scintilla, SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        call(scintilla, SCI_SETFIRSTVISIBLELINE, line, 0U);
        enabled = false;
    }
    else
    {
        bool proceed = true;
        int size = call(scintilla, SCI_GETLENGTH, 0U, 0U);
        bool documentBig = size >= (1 << 19);
        if (documentBig && !confirmedOnce)
        {
            int width = call(scintilla, SCI_GETSCROLLWIDTH, 0U, 0U);
            proceed = confirmedOnce = confirmWrap(width);
        }
        if (proceed)
        {
            call(scintilla, SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
            enabled = true;
        }
    }
}

bool WrapState::defConfirmWrap(int width)
{
    auto &&text = fmt::format("This document is quite large and the longest of its lines is at least {} characters long.\nAre you sure you want to enable line wrapping?", width);
    return messageBox(text, mfInformation | mfYesButton | mfNoButton) == cmYes;
}

/////////////////////////////////////////////////////////////////////////
// AutoIndent

void AutoIndent::applyToCurrentLine(TScintilla &scintilla)
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
// ThemingState

void ThemingState::apply(TScintilla &scintilla) const
{
    auto &scheme = getScheme();
    setStyleColor(scintilla, STYLE_DEFAULT, scheme[sNormal]);
    call(scintilla, SCI_STYLECLEARALL, 0U, 0U); // Must be done before setting other colors.
    setSelectionColor(scintilla, scheme[sSelection]);
    setWhitespaceColor(scintilla, scheme[sWhitespace]);
    setStyleColor(scintilla, STYLE_CONTROLCHAR, normalize(scheme, sCtrlChar));
    setStyleColor(scintilla, STYLE_LINENUMBER, normalize(scheme, sLineNums));
    if (lexerInfo)
    {
        call(scintilla, SCI_SETLEXER, lexerInfo->lexerId, 0U);
        for (const auto &s : lexerInfo->styles)
            setStyleColor(scintilla, s.id, normalize(scheme, s.style));
        for (const auto &k : lexerInfo->keywords)
            call(scintilla, SCI_SETKEYWORDS, k.id, (sptr_t) k.keywords);
        for (const auto &p : lexerInfo->properties)
            call(scintilla, SCI_SETPROPERTY, (sptr_t) p.name, (sptr_t) p.value);
    }
    else
        call(scintilla, SCI_SETLEXER, SCLEX_CONTAINER, 0U);
    call(scintilla, SCI_COLOURISE, 0, -1);
}

static bool isBrace(TStringView braces, char ch)
{
    return memchr(braces.data(), ch, braces.size()) != nullptr;
}

void ThemingState::updateBraces(TScintilla &scintilla) const
{
    if (lexerInfo)
    {
        auto pos = call(scintilla, SCI_GETCURRENTPOS, 0U, 0U);
        auto ch = call(scintilla, SCI_GETCHARAT, pos, 0U);
        bool braceFound = false;
        if (isBrace(lexerInfo->braces, ch))
        {
            // We must lex any newly inserted text so that it has the right style.
            idleWork(scintilla);
            // Scintilla already makes sure that both braces have the same style.
            auto matchPos = call(scintilla, SCI_BRACEMATCH, pos, 0U);
            if (matchPos != -1)
            {
                auto &scheme = getScheme();
                auto style = call(scintilla, SCI_GETSTYLEAT, pos, 0U);
                auto curAttr = getStyleColor(scintilla, style);
                auto braceAttr = coalesce(scheme[sBraceMatch], curAttr);
                setStyleColor(scintilla, STYLE_BRACELIGHT, braceAttr);
                call(scintilla, SCI_BRACEHIGHLIGHT, pos, matchPos);
                braceFound = true;
            }
        }
        if (!braceFound)
            call(scintilla, SCI_BRACEHIGHLIGHT, -1, -1);
    }
}

/////////////////////////////////////////////////////////////////////////

void stripTrailingSpaces(TScintilla &scintilla)
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

void ensureNewlineAtEnd(TScintilla &scintilla)
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
