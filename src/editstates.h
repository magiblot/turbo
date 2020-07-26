#ifndef TVEDIT_EDITSTATES_H
#define TVEDIT_EDITSTATES_H

#define Uses_MsgBox
#include <tvision/tv.h>

#include <ScintillaHeaders.h>
#include <tuple>
#include <fmt/core.h>

class LineNumbersWidth {

    const int minWidth;
    int lastWidth {0};

public:

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    std::tuple<int, int> update(Scintilla::TScintillaEditor &editor)
    {
        size_t newLines = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
        int newWidth = 1;
        while (newLines /= 10)
            ++newWidth;
        if (newWidth < minWidth)
            newWidth = minWidth;
        if (newWidth != lastWidth) {
            int delta = newWidth - lastWidth;
            lastWidth = newWidth;
            return {lastWidth, delta};
        }
        return {lastWidth, 0};
    }

};

class WrapMode {

    bool wrapEnabled {false};
    bool confirmedOnce {false};

public:

    bool toggle(Scintilla::TScintillaEditor &editor, bool dialog=true)
    {
        bool proceed = true;
        if (wrapEnabled)
            editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        else {
            int width = editor.WndProc(SCI_GETSCROLLWIDTH, 0U, 0U);
            if (width > 4096 && !confirmedOnce) {
                if (dialog) {
                    auto &&text = fmt::format("The longest line in this document is at least {} characters long.\nAre you sure you want to enable line wrapping?", width);
                    ushort res = messageBox(text.c_str(), mfInformation | mfYesButton | mfNoButton);
                    if (res == cmYes)
                        proceed = confirmedOnce = true;
                    else
                        proceed = false;
                } else
                    proceed = false;
            }
            if (proceed)
                editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
        }
        wrapEnabled = !wrapEnabled;
        return proceed;
    }

    bool enabled() const {
        return wrapEnabled;
    }

};

#endif
