#ifndef TURBO_SCINTILLA_H
#define TURBO_SCINTILLA_H

#include "scintilla/include/Scintilla.h"

class TPoint;
class TRect;
class TDrawSurface;
struct TColorAttr;
struct KeyDownEvent;
struct MouseEventType;

namespace Scintilla {
class TScintilla;
} // namespace Scintilla

namespace turbo {

class Clipboard;
using TScintilla = Scintilla::TScintilla;
class TScintillaParent;

TScintilla &createScintilla(Clipboard *aClipboard) noexcept;
void destroyScintilla(TScintilla &) noexcept;

sptr_t call(TScintilla &, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

void setParent(TScintilla &, TScintillaParent *aParent);
void changeSize(TScintilla &);
void clearBeforeTentativeStart(TScintilla &);
void insertPasteStream(TScintilla &, TStringView text);
void insertCharacter(TScintilla &, TStringView mbc);
void idleWork(TScintilla &);
TPoint pointMainCaret(TScintilla &);
bool handleKeyDown(TScintilla &, const KeyDownEvent &keyDown);
bool handleMouse(TScintilla &, unsigned short what, const MouseEventType &mouse);
void paint(TScintilla &, TDrawSurface &surface, TRect area);
void setStyleColor(TScintilla &, int style, TColorAttr attr);
void setSelectionColor(TScintilla &, TColorAttr attr);
void setWhitespaceColor(TScintilla &, TColorAttr attr);

class TScintillaParent
{
public:
    virtual TPoint getEditorSize() noexcept;
    virtual void invalidate(TRect area) noexcept;
    virtual void handleNotification(const SCNotification &scn);
    virtual void setVerticalScrollPos(int delta, int limit) noexcept;
    virtual void setHorizontalScrollPos(int delta, int limit) noexcept;
};

} // namespace turbo

#endif // TURBO_SCINTILLA_H
