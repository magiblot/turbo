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
public:

    ::Scintilla::SelectionText &selText;

    Clipboard() noexcept;
    ~Clipboard();

    virtual void xSetText(TStringView) noexcept = 0;
    virtual void xGetText(TFuncView<void(bool, TStringView)> accept) noexcept = 0;

};

class LcbClipboard : public Clipboard
{
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
