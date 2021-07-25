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
class ScintillaTV;
} // namespace Scintilla

namespace turbo {

class Clipboard;
class ScintillaParent;
using Scintilla = ::Scintilla::ScintillaTV;

Scintilla &createScintilla(Clipboard *aClipboard) noexcept;
void destroyScintilla(Scintilla &) noexcept;

sptr_t call(Scintilla &, unsigned int iMessage, uptr_t wParam, sptr_t lParam);

void setParent(Scintilla &, ScintillaParent *aParent);
void changeSize(Scintilla &);
void clearBeforeTentativeStart(Scintilla &);
void insertPasteStream(Scintilla &, TStringView text);
void insertCharacter(Scintilla &, TStringView mbc);
void idleWork(Scintilla &);
TPoint pointMainCaret(Scintilla &);
bool handleKeyDown(Scintilla &, const KeyDownEvent &keyDown);
bool handleMouse(Scintilla &, unsigned short what, const MouseEventType &mouse);
void paint(Scintilla &, TDrawSurface &surface, TRect area);
void setStyleColor(Scintilla &, int style, TColorAttr attr);
TColorAttr getStyleColor(Scintilla &, int style);
void setSelectionColor(Scintilla &, TColorAttr attr);
void setWhitespaceColor(Scintilla &, TColorAttr attr);

class ScintillaParent
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
