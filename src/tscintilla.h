#ifndef TURBO_TSCINTILLA_H
#define TURBO_TSCINTILLA_H

#define Uses_TKeys
#define Uses_TView
#define Uses_TEvent
#include <tvision/tv.h>

#include <ScintillaHeaders.h>
#include <string_view>

#include "clipboard.h"

struct DocumentView;
struct EditorSurface;

namespace Scintilla {

class TScintillaWindow;

struct TScintillaEditor : public ScintillaBase {

    friend struct DocumentView;

    TScintillaWindow *parent {0};
    Clipboard *clipboard;

    TScintillaEditor();

    void SetVerticalScrollPos() override;
    void SetHorizontalScrollPos() override;
    bool ModifyScrollBars(Sci::Line nMax, Sci::Line nPage) override;
    void Copy() override;
    void Paste() override;
    void ClaimSelection() override;
    void NotifyChange() override;
    void NotifyParent(SCNotification scn) override;
    void NotifyStyleToNeeded(Sci::Position endStyleNeeded) override;
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
    void draw(EditorSurface &surface);
    void setStyleColor(int style, TColorAttr attr);
    void setSelectionColor(TColorAttr attr);
    void setWhitespaceColor(TColorAttr attr);
    static void drawWrapMarker(Surface *, PRectangle, bool, ColourDesired);

    void setWindow(EditorSurface *wid);
    void setParent(TScintillaWindow *parent_);
    void changeSize();
    void clearBeforeTentativeStart();
    void pasteText(std::string_view text);
    void insertCharacter(std::string_view ch);
    void idleWork();
    Sci::Line getFirstVisibleDocumentLine();
    TPoint getCaretPosition();
    TPoint getDelta();

};

inline void TScintillaEditor::setWindow(EditorSurface *wid)
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

class TScintillaWindow {

public:

    virtual void notify(SCNotification scn) {};
    virtual void setVerticalScrollPos(int delta, int limit) = 0;
    virtual void setHorizontalScrollPos(int delta, int limit) = 0;
    virtual void notifyStyleToNeeded(Sci::Position endStyleNeeded) {};

};

}

#endif
