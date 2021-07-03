#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <chrono>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

#include "tscintilla.h"

using namespace Scintilla;

#include "surface.h"

TScintillaEditor::TScintillaEditor()
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
}

void TScintillaEditor::SetVerticalScrollPos()
{
    auto *parent = getParent();
    if (parent) {
        auto limit = LinesOnScreen() + MaxScrollPos();
        parent->setVerticalScrollPos(topLine, limit);
    }
}

void TScintillaEditor::SetHorizontalScrollPos()
{
    auto *parent = getParent();
    if (parent)
        parent->setHorizontalScrollPos(xOffset, vs.wrapState == SC_WRAP_NONE ? scrollWidth : 1);
}

bool TScintillaEditor::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage)
{
    SetVerticalScrollPos();
    SetHorizontalScrollPos();
    return false;
}

// Clipboard actions copied from ScinTerm.

void TScintillaEditor::Copy()
{
    if (clipboard && !sel.Empty()) {
        clipboard->copy(
            [this] (auto &selText) {
                CopySelectionRange(&selText);
            }
        );
    }
}

void TScintillaEditor::Paste()
{
    if (clipboard) {
        clipboard->paste(
            [this] (auto &selText, auto text) {
                if (text.size()) {
                    ClearSelection(multiPasteMode == SC_MULTIPASTE_EACH);
                    selText.Copy( text,
                                  pdoc->dbcsCodePage,
                                  vs.styles[STYLE_DEFAULT].characterSet,
                                  false,
                                  true );
                    InsertPasteShape( selText.Data(),
                                      selText.Length(),
                                      selText.rectangular ? pasteRectangular
                                                          : pasteStream );
                    EnsureCaretVisible();
                }
            }
        );
    }
}

void TScintillaEditor::ClaimSelection()
{
}

void TScintillaEditor::NotifyChange()
{
}

void TScintillaEditor::NotifyParent(SCNotification scn)
{
    auto *parent = getParent();
    if (parent)
        parent->handleNotification(scn);
}

void TScintillaEditor::CopyToClipboard(const SelectionText &selectedText)
{
}

bool TScintillaEditor::FineTickerRunning(TickReason reason)
{
    return false;
}

void TScintillaEditor::FineTickerStart(TickReason reason, int millis, int tolerance)
{
}

void TScintillaEditor::FineTickerCancel(TickReason reason)
{
}

void TScintillaEditor::SetMouseCapture(bool on)
{
}

bool TScintillaEditor::HaveMouseCapture()
{
    return true;
}

sptr_t TScintillaEditor::DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam)
{
    return 0;
}

void TScintillaEditor::CreateCallTipWindow(PRectangle rc)
{
}

void TScintillaEditor::AddToPopUp(const char *label, int cmd, bool enabled)
{
}

CaseFolder *TScintillaEditor::CaseFolderForEncoding()
{
    if (IsUnicodeMode())
        return new CaseFolderUnicode();
    return ScintillaBase::CaseFolderForEncoding();
}

int TScintillaEditor::KeyDefault(int key, int modifiers) {
    if (!modifiers) {
        Editor::AddChar(key);
        return 1;
    }
    return 0;
}

int TScintillaEditor::convertModifiers(ulong controlKeyState)
{
    struct { ushort tv; int scmod; } static constexpr modifiersTable[] = {
        {kbShift,       SCMOD_SHIFT},
        {kbCtrlShift,   SCMOD_CTRL},
        {kbAltShift,    SCMOD_ALT}
    };

    int modifiers = 0;
    for (const auto [tv, scmod] : modifiersTable)
        if (controlKeyState & tv)
            modifiers |= scmod;
    return modifiers;
}

void TScintillaEditor::KeyDownWithModifiers(const KeyDownEvent &keyDown, bool *consumed)
{
    struct { ushort tv; int sck; } static constexpr keysTable[] = {
        {kbDown,        SCK_DOWN},
        {kbUp,          SCK_UP},
        {kbLeft,        SCK_LEFT},
        {kbRight,       SCK_RIGHT},
        {kbHome,        SCK_HOME},
        {kbEnd,         SCK_END},
        {kbPgUp,        SCK_PRIOR},
        {kbPgDn,        SCK_NEXT},
        {kbDel,         SCK_DELETE},
        {kbIns,         SCK_INSERT},
        {kbTab,         SCK_TAB},
        {kbEnter,       SCK_RETURN},
        {kbBack,        SCK_BACK},
        {kbShiftDel,    SCK_DELETE},
        {kbShiftIns,    SCK_INSERT},
        {kbShiftTab,    SCK_TAB},
        {kbCtrlDown,    SCK_DOWN},
        {kbCtrlUp,      SCK_UP},
        {kbCtrlLeft,    SCK_LEFT},
        {kbCtrlRight,   SCK_RIGHT},
        {kbCtrlHome,    SCK_HOME},
        {kbCtrlEnd,     SCK_END},
        {kbCtrlPgUp,    SCK_PRIOR},
        {kbCtrlPgDn,    SCK_NEXT},
        {kbCtrlDel,     SCK_DELETE},
        {kbCtrlIns,     SCK_INSERT},
        {kbCtrlEnter,   SCK_RETURN},
        {kbCtrlBack,    SCK_BACK},
        {kbAltDown,     SCK_DOWN},
        {kbAltUp,       SCK_UP},
        {kbAltLeft,     SCK_LEFT},
        {kbAltRight,    SCK_RIGHT},
        {kbAltHome,     SCK_HOME},
        {kbAltEnd,      SCK_END},
        {kbAltPgUp,     SCK_PRIOR},
        {kbAltPgDn,     SCK_NEXT},
        {kbAltDel,      SCK_DELETE},
        {kbAltIns,      SCK_INSERT},
        {kbAltBack,     SCK_BACK},
    };

    int modifiers = convertModifiers(keyDown.controlKeyState);
    bool specialKey = modifiers && !keyDown.textLength;

    int key;
    if (keyDown.keyCode <= kbCtrlZ)
        key = keyDown.keyCode + 'A' - 1;
    else {
        key = keyDown.charScan.charCode;
        for (const auto [tv, sck] : keysTable)
            if (keyDown.keyCode == tv) {
                key = sck;
                specialKey = true;
                break;
            }
    }

    if (specialKey)
        ScintillaBase::KeyDownWithModifiers(key, modifiers, consumed);
    else
        ScintillaBase::InsertCharacter({keyDown.text, keyDown.textLength}, CharacterSource::directInput);
}

bool TScintillaEditor::MouseEvent(const TEvent &ev) {
    auto where = ev.mouse.where;
    auto pt = Point::FromInts(where.x, where.y);
    uint time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    int modifiers = convertModifiers(ev.mouse.controlKeyState); // This will actually be 0.
    if (ev.mouse.buttons & mbLeftButton) {
        // Scintilla actually assumes these functions are invoked only for the
        // left button mouse. Note that the original Turbo Vision does not set
        // the 'buttons' mask for evMouseUp.
        switch (ev.what) {
            case evMouseDown:
                Editor::ButtonDownWithModifiers(pt, time, modifiers);
                break;
            case evMouseUp:
                Editor::ButtonUpWithModifiers(pt, time, modifiers);
                break;
            case evMouseMove:
            case evMouseAuto:
                Editor::ButtonMoveWithModifiers(pt, time, modifiers);
                break;
        }
        return true;
    }
    return false;
}

void TScintillaEditor::paint(TDrawSurface &d)
// 'd.size' should equal 'parent->editorSize()'.
{
    TScintillaSurface s;
    s.surface = &d;
    s.defaultTextAttr = getStyleColor(STYLE_DEFAULT);
    Editor::Paint(&s, PRectangle::FromInts(0, 0, d.size.x, d.size.y));
}

void TScintillaEditor::setStyleColor(int style, TColorAttr attr)
{
    WndProc(SCI_STYLESETFORE, style, convertColor(::getFore(attr)).AsInteger());
    WndProc(SCI_STYLESETBACK, style, convertColor(::getBack(attr)).AsInteger());
    WndProc(SCI_STYLESETWEIGHT, style, ::getStyle(attr));
}

TColorAttr TScintillaEditor::getStyleColor(int style)
{
    ColourDesired fore {(int) WndProc(SCI_STYLEGETFORE, style, 0U)};
    ColourDesired back {(int) WndProc(SCI_STYLEGETBACK, style, 0U)};
    auto styleWeight = WndProc(SCI_STYLEGETWEIGHT, style, 0U);
    return {
        convertColor(fore),
        convertColor(back),
        (ushort) styleWeight,
    };
}

void TScintillaEditor::setSelectionColor(TColorAttr attr)
{
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    WndProc(SCI_SETSELFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    WndProc(SCI_SETSELBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

void TScintillaEditor::setWhitespaceColor(TColorAttr attr)
{
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    WndProc(SCI_SETWHITESPACEFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    WndProc(SCI_SETWHITESPACEBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

void TScintillaEditor::drawWrapMarker(Surface *surface, PRectangle rcPlace, bool isEndMarker, ColourDesired wrapColour)
{
    auto *s = (TScintillaSurface *) surface;
    Font f {};
    if (isEndMarker) {
        // Imitate the Tilde text editor.
        s->DrawTextTransparent(rcPlace, f, rcPlace.bottom, "↵", wrapColour);
    }
}

TPoint TScintillaParent::getEditorSize()
{
    return {0, 0};
}

void TScintillaParent::handleNotification(const SCNotification &scn)
{
}

void TScintillaParent::setVerticalScrollPos(int, int)
{
}

void TScintillaParent::setHorizontalScrollPos(int, int)
{
}
