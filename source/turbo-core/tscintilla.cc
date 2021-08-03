#include <turbo/scintilla.h>
#include <turbo/scintilla/tscintilla.h>
#include <turbo/clipboard.h>

#include "platform/surface.h"

namespace Scintilla {

TScintilla::TScintilla(turbo::Clipboard *aClipboard) :
    clipboard(aClipboard)
{
    // Block caret for both Insertion and Overwrite mode.
    WndProc(SCI_SETCARETSTYLE, CARETSTYLE_BLOCK | CARETSTYLE_OVERSTRIKE_BLOCK, 0U);
    // Disable margin on line numbers.
    vs.marginNumberPadding = 0;
    // Disable margin pixels
    WndProc(SCI_SETMARGINLEFT, 0U, 0);
    WndProc(SCI_SETMARGINRIGHT, 0U, 0);
    // Disable buffered fraw
    WndProc(SCI_SETBUFFEREDDRAW, 0, 0U);
    // Disable space between lines
    WndProc(SCI_SETEXTRADESCENT, -1, 0U);
    // Stay in Unicode mode (experimental).
//     WndProc(SCI_SETCODEPAGE, SC_CHARSET_ANSI, 0U);
//     WndProc(SCI_STYLESETCHARACTERSET, STYLE_DEFAULT, SC_CHARSET_ANSI);
    // Set our custom representations.
    reprs.Clear();
    {
        constexpr int ranges[][2] = {{0, ' '}, {0x7F, 0x100}};
        for (auto [beg, end] : ranges) {
            for (int i = beg; i < end; ++i) {
                char c[2] = {(char) i};
                char r[8] = {};
                sprintf(r, "\\x%02X", i);
                reprs.SetRepresentation(c, r);
            }
        }
        reprs.SetRepresentation("\t", "»        ");
    }
    // Do not use padding for control characters.
    vs.ctrlCharPadding = 0;
    view.tabWidthMinimumPixels = 0; // Otherwise, tabs will be more than 8 columns wide.
    // Always draw tabulators.
    WndProc(SCI_SETVIEWWS, SCWS_VISIBLEALWAYS, 0U);
    // Process mouse down events:
    WndProc(SCI_SETMOUSEDOWNCAPTURES, true, 0U);
    // Double clicks only in the same cell.
    doubleClickCloseThreshold = Point(0, 0);
    // Set our custom function to draw wrap markers.
    view.customDrawWrapMarker = drawWrapMarker;

    // Extra key shortcuts
    WndProc(SCI_ASSIGNCMDKEY, SCK_UP | ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_MOVESELECTEDLINESUP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_DOWN | ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_MOVESELECTEDLINESDOWN);
    // Since Ctrl+Back won't work in most terminals, allow Alt+Back, like Bash.
    WndProc(SCI_ASSIGNCMDKEY, SCK_BACK | ((SCMOD_ALT) << 16), SCI_DELWORDLEFT);
    // Ctrl+Shift+Z won't work in most terminals.
    WndProc(SCI_ASSIGNCMDKEY, 'Z' | ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_REDO);
    // Home/End keys should respect line wrapping.
    WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_NORM << 16), SCI_VCHOMEWRAP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_SHIFT << 16), SCI_VCHOMEWRAPEXTEND);
    WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_NORM << 16), SCI_LINEENDWRAP);
    WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_SHIFT << 16), SCI_LINEENDWRAPEXTEND);
}

void TScintilla::SetVerticalScrollPos()
{
    auto *parent = getParent();
    if (parent) {
        auto limit = LinesOnScreen() + MaxScrollPos();
        parent->setVerticalScrollPos(topLine, limit);
    }
}

void TScintilla::SetHorizontalScrollPos()
{
    auto *parent = getParent();
    if (parent)
        parent->setHorizontalScrollPos(xOffset, vs.wrapState == SC_WRAP_NONE ? scrollWidth : 1);
}

bool TScintilla::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage)
{
    SetVerticalScrollPos();
    SetHorizontalScrollPos();
    return false;
}

// Clipboard actions copied from ScinTerm.

void TScintilla::Copy()
{
    if (clipboard && !sel.Empty())
        clipboard->setSelection([&] (auto &selText) {
            CopySelectionRange(&selText);
        });
}

void TScintilla::Paste()
{
    if (clipboard)
        clipboard->getSelection([&] (const auto &selText) {
            if (selText.Length())
            {
                ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);
                InsertPasteShape( selText.Data(),
                                  selText.Length(),
                                  selText.rectangular ? pasteRectangular
                                                      : pasteStream );
                EnsureCaretVisible();
            }
        });
}

void TScintilla::ClaimSelection()
{
}

void TScintilla::NotifyChange()
{
}

void TScintilla::NotifyParent(SCNotification scn)
{
    auto *parent = getParent();
    if (parent)
        parent->handleNotification(scn);
}

void TScintilla::CopyToClipboard(const SelectionText &selectedText)
{
}

bool TScintilla::FineTickerRunning(TickReason reason)
{
    return false;
}

void TScintilla::FineTickerStart(TickReason reason, int millis, int tolerance)
{
}

void TScintilla::FineTickerCancel(TickReason reason)
{
}

void TScintilla::SetMouseCapture(bool on)
{
}

bool TScintilla::HaveMouseCapture()
{
    return true;
}

sptr_t TScintilla::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return 0;
}

void TScintilla::CreateCallTipWindow(PRectangle rc)
{
}

void TScintilla::AddToPopUp(const char *label, int cmd, bool enabled)
{
}

CaseFolder *TScintilla::CaseFolderForEncoding()
{
    if (IsUnicodeMode())
        return new CaseFolderUnicode();
    return super::CaseFolderForEncoding();
}

int TScintilla::KeyDefault(int key, int modifiers) {
    if (!modifiers)
    {
        super::AddChar(key);
        return 1;
    }
    return 0;
}

void TScintilla::drawWrapMarker(Surface *surface, PRectangle rcPlace, bool isEndMarker, ColourDesired wrapColour)
{
    auto *s = (TScintillaSurface *) surface;
    Font f {};
    if (isEndMarker)
        // Imitate the Tilde text editor.
        s->DrawTextTransparent(rcPlace, f, rcPlace.bottom, "↵", wrapColour);
}

} // namespace Scintilla
