#ifndef TURBO_CLIPBOARD_H
#define TURBO_CLIPBOARD_H

#include <tvision/tv.h>
#include <turbo/funcview.h>
#include <stdio.h>

struct clipboard_c;

namespace Scintilla {
class SelectionText;
} // namespace Scintilla

namespace turbo {

class Clipboard
{
    // 'Clipboard' allows for sharing a common clipboard state between
    // 'TScintilla' instances.

    // This keeps a local copy of the clipboard's contents even if 'xSetText' or
    // 'xGetText' don't work. You needn't touch this; it's managed by 'TScintilla'.
    Scintilla::SelectionText &selText;

public:

    Clipboard() noexcept;
    ~Clipboard();

    void setText(TStringView text) noexcept;
    void getText(TFuncView<void(TStringView text)> accept) noexcept;

    // To be used by TScintilla.
    void setSelection(TFuncView<void(Scintilla::SelectionText &)> fill) noexcept;
    void getSelection(TFuncView<void(const Scintilla::SelectionText &)> accept) noexcept;

protected:

    // Called when the clipboard's contents change.
    virtual void xSetText(TStringView) noexcept = 0;
    // Called when retrieving new clipboard contents.
    virtual void xGetText(TFuncView<void(bool ok, TStringView text)> accept) noexcept = 0;

};

class SystemClipboard : public Clipboard
{
    // This implementation of 'Clipboard' interacts with the system clipboard
    // (Win32/X11/Cocoa).

    struct clipboard_c *cb;
#ifndef _WIN32
    FILE *ttyout;
#endif

public:

    SystemClipboard() noexcept;
    ~SystemClipboard();

    void xSetText(TStringView) noexcept override;
    void xGetText(TFuncView<void(bool, TStringView)> accept) noexcept override;
};

} // namespace turbo

#endif // TURBO_CLIPBOARD_H
