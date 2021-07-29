#include <turbo/clipboard.h>
#include <libclipboard.h>
#include <turbo/scintilla/internals.h>

namespace turbo {

Clipboard::Clipboard() noexcept :
    selText(*new Scintilla::SelectionText)
{
}

Clipboard::~Clipboard()
{
    delete &selText;
}

SystemClipboard::SystemClipboard() noexcept
{
    cb = clipboard_new(nullptr);
}

SystemClipboard::~SystemClipboard()
{
    clipboard_free(cb);
}

void SystemClipboard::xSetText(TStringView text) noexcept
{
    if (cb)
        clipboard_set_text_ex(cb, text.data(), (int) text.size(), LCB_CLIPBOARD);
}

void SystemClipboard::xGetText(TFuncView<void(bool, TStringView)> accept) noexcept
{
    char *text = cb ? clipboard_text(cb) : nullptr;
    accept(text != nullptr, text);
    ::free(text);
}

} // namespace turbo
