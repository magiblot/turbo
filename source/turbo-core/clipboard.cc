#include <turbo/clipboard.h>
#include <libclipboard.h>

namespace turbo {

LcbClipboard::LcbClipboard() noexcept
{
    cb = clipboard_new(nullptr);
}

LcbClipboard::~LcbClipboard() noexcept
{
    clipboard_free(cb);
}

void LcbClipboard::xSetText(TStringView text) noexcept
{
    if (cb)
        clipboard_set_text_ex(cb, text.data(), (int) text.size(), LCB_CLIPBOARD);
}

void LcbClipboard::xGetText(TFuncView<void(bool, TStringView)> accept) noexcept
{
    char *text = cb ? clipboard_text(cb) : nullptr;
    accept(text != nullptr, text);
    ::free(text);
}

} // namespace turbo
