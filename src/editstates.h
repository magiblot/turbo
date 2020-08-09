#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#define Uses_MsgBox
#include <tvision/tv.h>

#include <ScintillaHeaders.h>
#include <tuple>
#include <fmt/core.h>

#include "tscintilla.h"

class LineNumbersWidth {

    const int minWidth;
    int lastWidth {0};
    bool enabled {false};

public:

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    void toggle()
    {
        setState(!enabled);
    }

    void setState(bool enable)
    {
        enabled = enable;
    }

    bool isEnabled() const
    {
        return enabled;
    }

    std::tuple<int, int> update(Scintilla::TScintillaEditor &editor)
    {
        int newWidth;
        if (enabled) {
            newWidth = 1;
            size_t newLines = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
            while (newLines /= 10)
                ++newWidth;
            if (newWidth < minWidth)
                newWidth = minWidth;
        } else
            newWidth = 0;
        if (newWidth != lastWidth) {
            int delta = newWidth - lastWidth;
            lastWidth = newWidth;
            return {newWidth, delta};
        }
        return {newWidth, 0};
    }

};

class WrapMode {

    bool wrapEnabled {false};
    bool confirmedOnce {false};

public:

    bool toggle(Scintilla::TScintillaEditor &editor, bool dialog=true)
    {
        bool proceed = true;
        if (wrapEnabled) {
            auto line = editor.getFirstVisibleDocumentLine();
            editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
            editor.WndProc(SCI_SETFIRSTVISIBLELINE, line, 0U);
            wrapEnabled = false;
        }
        else {
            const int width = editor.WndProc(SCI_GETSCROLLWIDTH, 0U, 0U);
            const int size = editor.WndProc(SCI_GETLENGTH, 0U, 0U);
            const bool documentBig = size >= (1 << 19) && width > 512;
            if (documentBig && !confirmedOnce) {
                if (dialog) {
                    auto &&text = fmt::format("This document is very big and the longest of its lines is at least {} characters long.\nAre you sure you want to enable line wrapping?", width);
                    ushort res = messageBox(text.c_str(), mfInformation | mfYesButton | mfNoButton);
                    if (res == cmYes)
                        proceed = confirmedOnce = true;
                    else
                        proceed = false;
                } else
                    proceed = false;
            }
            if (proceed) {
                editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_WORD, 0U);
                wrapEnabled = true;
            }
        }
        return proceed;
    }

    bool enabled() const {
        return wrapEnabled;
    }

};

struct EditorWindow;

struct FileType {

    void detect(EditorWindow &win);

};

struct Indent {

    bool autoIndent {true};

    void toggle()
    {
        autoIndent = !autoIndent;
    }

    void autoIndentCurrentLine(Scintilla::TScintillaEditor &editor)
    {
        if (autoIndent) {
            auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
            auto line = editor.WndProc(SCI_LINEFROMPOSITION, pos, 0U);
            if (line > 0) {
                auto indentation = editor.WndProc(SCI_GETLINEINDENTATION, line - 1, 0U);
                if (indentation > 0) {
                    editor.WndProc(SCI_SETLINEINDENTATION, line, indentation);
                    editor.WndProc(SCI_VCHOME, 0U, 0U);
                }
            }
        }
    }

};

struct DocumentProperties {

    static constexpr uint
        ndEOL = 0x0001;

    uint notDetected;
    int eolType;

    DocumentProperties()
    {
        reset();
    }

    void reset()
    {
        notDetected = ndEOL;
        eolType = SC_EOL_LF; // Default EOL type is LF.
    }

    void analyze(std::string_view text)
    {
        if (text.size()) {
            char cur = text[0];
            char next = text.size() > 0 ? text[1] : '\0';
            int i = 1;
            while (notDetected) {
                if (notDetected & ndEOL) {
                    if (cur == '\r' && next == '\n')
                        eolType = SC_EOL_CRLF, notDetected &= ~ndEOL;
                    else if (cur == '\n')
                        eolType = SC_EOL_LF, notDetected &= ~ndEOL;
                    else if (cur == '\r')
                        eolType = SC_EOL_CR, notDetected &= ~ndEOL;
                }
                if (++i < text.size()) {
                    cur = next;
                    next = text[i];
                } else
                    break;
            };
        }
    }

    void apply(Scintilla::TScintillaEditor &editor) const
    {
        editor.WndProc(SCI_SETEOLMODE, eolType, 0U);
    }

};

#endif
