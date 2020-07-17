#include "tscintilla.h"

using namespace Scintilla;

#include "drawview.h"
#include "surface.h"

TScintillaEditor::TScintillaEditor()
{
    // To discern unused arguments.
    const sptr_t nil = 0;

    // Block caret for both Insertion and Overwrite mode.
    WndProc(SCI_SETCARETSTYLE, CARETSTYLE_BLOCK | CARETSTYLE_OVERSTRIKE_BLOCK, nil);
    // Show line numbers
    WndProc(SCI_SETMARGINS, 0, nil);
    WndProc(SCI_SETMARGINWIDTHN, 0, 5);
    // Disable margin pixels
    WndProc(SCI_SETMARGINLEFT, nil, 0);
    WndProc(SCI_SETMARGINRIGHT, nil, 0);
    // Disable buffered fraw
    WndProc(SCI_SETBUFFEREDDRAW, 0, nil);
    // Enable line wrapping
    WndProc(SCI_SETWRAPMODE, SC_WRAP_WORD, nil);
    // Disable space between lines
    WndProc(SCI_SETEXTRADESCENT, -1, nil);
    // Use single-byte character set.
    WndProc(SCI_SETCODEPAGE, SC_CHARSET_ANSI, nil);
    WndProc(SCI_STYLESETCHARACTERSET, STYLE_DEFAULT, SC_CHARSET_ANSI);
}

void TScintillaEditor::SetVerticalScrollPos()
{
}

void TScintillaEditor::SetHorizontalScrollPos()
{
}

bool TScintillaEditor::ModifyScrollBars(Sci::Line nMax, Sci::Line nPage)
{
    return false;
}

void TScintillaEditor::Copy()
{
}

void TScintillaEditor::Paste()
{
}

void TScintillaEditor::ClaimSelection()
{
}

void TScintillaEditor::NotifyChange()
{
}

void TScintillaEditor::NotifyParent(SCNotification scn)
{
    parent->notify(scn);
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
    return false;
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

int TScintillaEditor::KeyDefault(int key, int modifiers) {
    if (!modifiers) {
        Editor::AddChar(key);
        return 1;
    }
    return 0;
}

void TScintillaEditor::KeyDownWithModifiers(const KeyDownEvent &keyDown, bool *consumed)
{
    struct { ushort tv; int scmod; } static constexpr modifiersTable[] = {
        {kbShift,       SCMOD_SHIFT},
        {kbCtrlShift,   SCMOD_CTRL},
        {kbAltShift,    SCMOD_ALT}
    };

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
        {kbShiftDel,    SCK_DELETE},
        {kbShiftIns,    SCK_INSERT},
        {kbShiftTab,    '\t'},
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
        {kbCtrlBack,    '\b'}
    };

    int modifiers = 0;
    for (const auto [tv, scmod] : modifiersTable)
        if (keyDown.controlKeyState & tv)
            modifiers |= scmod;

    int key;
    if (keyDown.keyCode <= kbCtrlZ)
        key = keyDown.keyCode + 'A' - 1;
    else {
        key = keyDown.charScan.charCode;
        for (const auto [tv, sck] : keysTable)
            if (keyDown.keyCode == tv) {
                key = sck;
                break;
            }
    }

    Editor::KeyDownWithModifiers(key, modifiers, consumed);
}

void TScintillaEditor::draw(TDrawableView &d) {
    TScintillaSurface s;
    s.view = &d;
    Editor::Paint(&s, PRectangle(0, 0, d.size.x, d.size.y));
}

void TScintillaEditor::setStyleColor(int style, TCellAttribs attr)
{
    WndProc(SCI_STYLESETFORE, style, TScintillaSurface::convertColor(attr.colors.fg).AsInteger());
    WndProc(SCI_STYLESETBACK, style, TScintillaSurface::convertColor(attr.colors.bg).AsInteger());
}

void TScintillaEditor::setSelectionColor(TCellAttribs attr)
{
    WndProc(SCI_SETSELFORE, true, TScintillaSurface::convertColor(attr.colors.fg).AsInteger());
    WndProc(SCI_SETSELBACK, true, TScintillaSurface::convertColor(attr.colors.bg).AsInteger());
}
