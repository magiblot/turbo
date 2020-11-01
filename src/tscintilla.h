#ifndef TURBO_TSCINTILLA_H
#define TURBO_TSCINTILLA_H

#define Uses_TKeys
#define Uses_TView
#define Uses_TEvent
#include <tvision/tv.h>

#include <ScintillaHeaders.h>
#include <string_view>

struct DocumentView;
class TDrawSurface;

namespace Scintilla {

class TScintillaWindow;

struct TScintillaEditor : public ScintillaBase {

    friend struct DocumentView;

    TScintillaWindow *parent {0};
    SelectionText *clipboard;

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
    int convertModifiers(ulong controlKeyState);
    void KeyDownWithModifiers(const KeyDownEvent &keyDown, bool *consumed);
    bool MouseEvent(const TEvent &ev);
    void draw(TDrawSurface &drawView);
    void setStyleColor(int style, TCellAttribs attr);
    void setSelectionColor(TCellAttribs attr);
    void setWhitespaceColor(TCellAttribs attr);
    static void drawWrapMarker(Surface *, PRectangle, bool, ColourDesired);

    void setWindow(TDrawSurface *wid);
    void setParent(TScintillaWindow *parent_);
    void changeSize();
    void pasteText(std::string_view text);
    Sci::Line getFirstVisibleDocumentLine();
    TPoint getCaretPosition();
    TPoint getDelta();

};

inline void TScintillaEditor::setWindow(TDrawSurface *wid)
{
    wMain = wid;
}

inline void TScintillaEditor::setParent(TScintillaWindow *parent_)
{
    parent = parent_;
}

inline void TScintillaEditor::changeSize()
{
    ScintillaBase::ChangeSize();
}

inline void TScintillaEditor::pasteText(std::string_view text)
{
    InsertPasteShape(text.data(), text.size(), pasteStream);
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

class TScintillaWindow {

public:

    virtual void notify(SCNotification scn) {};
    virtual void setVerticalScrollPos(int delta, int limit) = 0;
    virtual void setHorizontalScrollPos(int delta, int limit) = 0;

};

}

#endif
