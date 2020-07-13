#include "tscintilla.h"

using namespace Scintilla;

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
}

void TScintillaEditor::CopyToClipboard(const SelectionText &selectedText)
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
    return false;
}

void TScintillaEditor::CreateCallTipWindow(PRectangle rc)
{
}

void TScintillaEditor::AddToPopUp(const char *label, int cmd, bool enabled)
{
}
