#ifndef TURBO_SCINTILLATV_H
#define TURBO_SCINTILLATV_H

#define Uses_TPoint
#include <tvision/tv.h>

#include <turbo/scintilla/ScintillaInternals.h>

namespace Scintilla {

struct ScintillaTV : ScintillaBase
{

    turbo::Clipboard *clipboard;

    ScintillaTV(turbo::Clipboard *aClipboard);

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

    void setParent(turbo::ScintillaParent *aParent);
    turbo::ScintillaParent *getParent() const;
    using ScintillaBase::ChangeSize;
    using ScintillaBase::ClearBeforeTentativeStart;
    using ScintillaBase::InsertPasteShape;
    using ScintillaBase::InsertCharacter;
    using ScintillaBase::IdleWork;
    using ScintillaBase::PointMainCaret;
    using ScintillaBase::KeyDownWithModifiers;
    using ScintillaBase::ButtonDownWithModifiers;
    using ScintillaBase::ButtonUpWithModifiers;
    using ScintillaBase::ButtonMoveWithModifiers;
    using ScintillaBase::Paint;
    using ScintillaBase::pasteStream;
    using ScintillaBase::CharacterSource;

    static void drawWrapMarker(Surface *, PRectangle, bool, ColourDesired);

};

inline void ScintillaTV::setParent(turbo::ScintillaParent *aParent)
{
    wMain = aParent;
}

inline turbo::ScintillaParent *ScintillaTV::getParent() const
{
    return (turbo::ScintillaParent *) wMain.GetID();
}

} // namespace Scintilla

#endif
