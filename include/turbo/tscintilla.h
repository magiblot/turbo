#ifndef TURBO_TSCINTILLA_H
#define TURBO_TSCINTILLA_H

#define Uses_TPoint
#include <tvision/tv.h>

#include <turbo/scintilla.h>
#include <string_view>

class TRect;
struct TEvent;
struct TColorAttr;
struct KeyDownEvent;
class TDrawSurface;
namespace turbo {
class Clipboard;
} // namespace turbo

namespace Scintilla {

struct TScintillaParent;

struct TScintillaEditor : public ScintillaBase {

    turbo::Clipboard *clipboard;

    TScintillaEditor(turbo::Clipboard *aClipboard = nullptr);

    void SetVerticalScrollPos() override;
    void SetHorizontalScrollPos() override;
    bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
    void Copy() override;
    void Paste() override;
    void ClaimSelection() override;
    void NotifyChange() override;
    void NotifyParent(SCNotification scn) override;
    void CopyToClipboard(const SelectionText &selectedText) override;
    bool FineTickerRunning(TickReason reason) override;
    void FineTickerStart(TickReason reason, int millis, int tolerance) override;
    void FineTickerCancel(TickReason reason) override;
    void SetMouseCapture(bool on) override;
    bool HaveMouseCapture() override;
    sptr_t DefWndProc(unsigned int iMessage, uptr_t wParam, sptr_t lParam) override;
    void CreateCallTipWindow(PRectangle rc) override;
    void AddToPopUp(const char *label, int cmd=0, bool enabled=true) override;

    CaseFolder *CaseFolderForEncoding() override;
    int KeyDefault(int key, int modifiers) override;
    int convertModifiers(ulong controlKeyState);
    void KeyDownWithModifiers(const KeyDownEvent &keyDown, bool *consumed);
    bool MouseEvent(const TEvent &ev);
    void paint(TDrawSurface &surface);
    void paint(TDrawSurface &surface, TRect area);
    void setStyleColor(int style, TColorAttr attr);
    TColorAttr getStyleColor(int style);
    void setSelectionColor(TColorAttr attr);
    void setWhitespaceColor(TColorAttr attr);
    static void drawWrapMarker(Surface *, PRectangle, bool, ColourDesired);

    void setParent(TScintillaParent *aParent);
    TScintillaParent *getParent() const;
    void changeSize();
    void clearBeforeTentativeStart();
    void pasteText(std::string_view text);
    void insertCharacter(std::string_view ch);
    void idleWork();
    Sci::Line getFirstVisibleDocumentLine();
    TPoint getCaretPosition();
    TPoint getDelta();

};

inline void TScintillaEditor::setParent(TScintillaParent *aParent)
{
    wMain = aParent;
}

inline TScintillaParent *TScintillaEditor::getParent() const
{
    return (TScintillaParent *) wMain.GetID();
}

inline void TScintillaEditor::changeSize()
{
    ScintillaBase::ChangeSize();
}

inline void TScintillaEditor::clearBeforeTentativeStart()
{
    ScintillaBase::ClearBeforeTentativeStart();
}

inline void TScintillaEditor::pasteText(std::string_view text)
{
    InsertPasteShape(text.data(), text.size(), pasteStream);
}

inline void TScintillaEditor::insertCharacter(std::string_view text)
{
    InsertCharacter(text, CharacterSource::directInput);
}

inline void TScintillaEditor::idleWork()
{
    IdleWork();
}

inline Sci::Line TScintillaEditor::getFirstVisibleDocumentLine()
{
    return pcs->DocFromDisplay(topLine);
}

inline TPoint TScintillaEditor::getCaretPosition()
{
    auto [x, y] = PointMainCaret();
    return {(int) x, (int) y};
}

inline TPoint TScintillaEditor::getDelta()
{
    return {xOffset, (int) topLine};
}

struct TScintillaParent {

    virtual TPoint getEditorSize() noexcept;
    virtual void invalidate(TRect area) noexcept;
    virtual void handleNotification(const SCNotification &scn);
    virtual void setVerticalScrollPos(int delta, int limit) noexcept;
    virtual void setHorizontalScrollPos(int delta, int limit) noexcept;

};

} // namespace Scintilla

#endif
