#define Uses_TKeys
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include <chrono>

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

template <class Func>
static inline void copy(turbo::Clipboard &self, Func &&fillSel) noexcept
{
    auto &selText = self.selText;
    fillSel(selText);
    self.xSetText({selText.Data(), selText.Length()});
}

void TScintilla::Copy()
{
    if (clipboard && !sel.Empty())
        copy(
            *clipboard,
            [&] (auto &selText) {
                CopySelectionRange(&selText);
            }
        );
}

template <class Func>
static inline void paste(turbo::Clipboard &self, Func &&fillSel) noexcept
{
    self.xGetText([&] (bool ok, TStringView text) {
        auto &selText = self.selText;
        fillSel(
            selText,
            ok ? text : TStringView {selText.Data(), selText.Length()}
        );
    });
}

void TScintilla::Paste()
{
    if (clipboard)
        paste(
            *clipboard,
            [&] (auto &selText, auto text) {
                if (text.size())
                {
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

namespace turbo {

TScintilla &createScintilla(Clipboard *aClipboard) noexcept
{
    return *new TScintilla(aClipboard);
}

void destroyScintilla(TScintilla &self) noexcept
{
    delete &self;
}

sptr_t call(TScintilla &self, uint iMessage, uptr_t wParam, sptr_t lParam)
{
    return self.WndProc(iMessage, wParam, lParam);
}

void setParent(TScintilla &self, TScintillaParent *aParent)
{
    self.setParent(aParent);
}

void changeSize(TScintilla &self)
{
    self.ChangeSize();
}

void clearBeforeTentativeStart(TScintilla &self)
{
    self.ClearBeforeTentativeStart();
}

void insertPasteStream(TScintilla &self, TStringView text)
{
    self.InsertPasteShape(text.data(), text.size(), TScintilla::pasteStream);
}

void insertCharacter(TScintilla &self, TStringView text)
{
    self.InsertCharacter(text, TScintilla::CharacterSource::directInput);
}

void idleWork(TScintilla &self)
{
    self.IdleWork();
}

TPoint pointMainCaret(TScintilla &self)
{
    auto p = self.PointMainCaret();
    return {(int) p.x, (int) p.y};
}

static int convertModifiers(ulong controlKeyState)
{
    static constexpr struct { ushort tv; int scmod; } modifiersTable[] =
    {
        {kbShift,       SCMOD_SHIFT},
        {kbCtrlShift,   SCMOD_CTRL},
        {kbAltShift,    SCMOD_ALT}
    };

    int modifiers = 0;
    for (const auto &m : modifiersTable)
        if (controlKeyState & m.tv)
            modifiers |= m.scmod;
    return modifiers;
}

bool handleKeyDown(TScintilla &self, const KeyDownEvent &keyDown)
{
    static constexpr struct { ushort tv; int sck; } keysTable[] =
    {
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
    else
    {
        key = keyDown.charScan.charCode;
        for (const auto [tv, sck] : keysTable)
            if (keyDown.keyCode == tv)
            {
                key = sck;
                specialKey = true;
                break;
            }
    }

    if (specialKey)
    {
        bool consumed = false;
        self.KeyDownWithModifiers(key, modifiers, &consumed);
        return consumed;
    }
    else
    {
        self.InsertCharacter({keyDown.text, keyDown.textLength}, TScintilla::CharacterSource::directInput);
        return true;
    }
}

bool handleMouse(TScintilla &self, ushort what, const MouseEventType &mouse)
{
    using namespace Scintilla;
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    auto pt = Point::FromInts(mouse.where.x, mouse.where.y);
    uint time = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    int modifiers = convertModifiers(mouse.controlKeyState); // Very few environments do support this.
    if (mouse.buttons & mbLeftButton)
    {
        // Scintilla actually assumes these functions are invoked only for the
        // left button mouse.
        switch (what)
        {
            case evMouseDown:
                self.ButtonDownWithModifiers(pt, time, modifiers);
                break;
            case evMouseUp:
                self.ButtonUpWithModifiers(pt, time, modifiers);
                break;
            case evMouseMove:
            case evMouseAuto:
                self.ButtonMoveWithModifiers(pt, time, modifiers);
                break;
        }
        return true;
    }
    return false;
}

static TColorAttr getStyleColor(TScintilla &self, int style)
{
    using namespace Scintilla;
    ColourDesired fore {(int) call(self, SCI_STYLEGETFORE, style, 0U)};
    ColourDesired back {(int) call(self, SCI_STYLEGETBACK, style, 0U)};
    auto styleWeight = call(self, SCI_STYLEGETWEIGHT, style, 0U);
    return {
        convertColor(fore),
        convertColor(back),
        (ushort) styleWeight,
    };
}

void paint(TScintilla &self, TDrawSurface &d, TRect area)
{
    using namespace Scintilla;
    TScintillaSurface s;
    s.surface = &d;
    s.defaultTextAttr = getStyleColor(self, STYLE_DEFAULT);
    self.Paint(
        &s,
        PRectangle::FromInts(area.a.x, area.a.y, area.b.x, area.b.y)
    );
}

void setStyleColor(TScintilla &self, int style, TColorAttr attr)
{
    using namespace Scintilla;
    call(self, SCI_STYLESETFORE, style, convertColor(::getFore(attr)).AsInteger());
    call(self, SCI_STYLESETBACK, style, convertColor(::getBack(attr)).AsInteger());
    call(self, SCI_STYLESETWEIGHT, style, ::getStyle(attr));
}

void setSelectionColor(TScintilla &self, TColorAttr attr)
{
    using namespace Scintilla;
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    call(self, SCI_SETSELFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    call(self, SCI_SETSELBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

void setWhitespaceColor(TScintilla &self, TColorAttr attr)
{
    using namespace Scintilla;
    auto fg = ::getFore(attr),
         bg = ::getBack(attr);
    call(self, SCI_SETWHITESPACEFORE, !fg.isDefault(), convertColor(fg).AsInteger());
    call(self, SCI_SETWHITESPACEBACK, !bg.isDefault(), convertColor(bg).AsInteger());
}

TPoint TScintillaParent::getEditorSize() noexcept
{
    return {0, 0};
}

void TScintillaParent::invalidate(TRect) noexcept
{
}

void TScintillaParent::handleNotification(const SCNotification &scn)
{
}

void TScintillaParent::setVerticalScrollPos(int, int) noexcept
{
}

void TScintillaParent::setHorizontalScrollPos(int, int) noexcept
{
}

} // namespace turbo
