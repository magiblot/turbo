#ifndef TVEDIT_TSCINTILLA_H
#define TVEDIT_TSCINTILLA_H

#define Uses_TKeys
#define Uses_TView
#define Uses_TEvent
#include <tvision/tv.h>

#include <ScintillaHeaders.h>

class DocumentView;
class TDrawableView;

namespace Scintilla {

struct TScintillaEditor : public ScintillaBase {

    friend class DocumentView;

    TScintillaEditor();

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

    int KeyDefault(int key, int modifiers) override;
    void KeyDownWithModifiers(const KeyDownEvent &keyDown, bool *consumed);
    void draw(TDrawableView &drawView);

    void setWindow(WindowID wid);
    TPoint getCaretPosition();

};

inline void TScintillaEditor::setWindow(WindowID wid)
{
    wMain = wid;
}

inline TPoint TScintillaEditor::getCaretPosition()
{
    auto [x, y] = PointMainCaret();
    return {(int) x, (int) y};
}

}

#endif
