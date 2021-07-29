#ifndef TURBO_CLIPBOARD_H
#define TURBO_CLIPBOARD_H

#include <tvision/tv.h>
#include <turbo/funcview.h>

struct clipboard_c;

namespace Scintilla {
class SelectionText;
} // namespace Scintilla

namespace turbo {

class Clipboard
{
    // 'Clipboard' allows for sharing a common clipboard state between
    // 'TScintilla' instances.
public:

    // This keeps a local copy of the clipboard's contents even if 'xSetText' or
    // 'xGetText' don't work. You needn't touch this; it's managed by 'TScintilla'.
    Scintilla::SelectionText &selText;

    Clipboard() noexcept;
    ~Clipboard();

    // Called when the clipboard's contents change.
    virtual void xSetText(TStringView) noexcept = 0;
    // Called when retrieving new clipboard contents.
    virtual void xGetText(TFuncView<void(bool, TStringView)> accept) noexcept = 0;

};

class LcbClipboard : public Clipboard
{
    // This implementation of 'Clipboard' interacts with the system clipboard
    // (Win32/X11/Cocoa).

    struct clipboard_c *cb;

public:

    LcbClipboard() noexcept;
    ~LcbClipboard();

    LcbClipboard(const LcbClipboard &) = delete;
    LcbClipboard& operator=(const LcbClipboard &) = delete;

    void xSetText(TStringView) noexcept override;
    void xGetText(TFuncView<void(bool, TStringView)> accept) noexcept override;

};

} // namespace turbo

#endif // TURBO_CLIPBOARD_H
