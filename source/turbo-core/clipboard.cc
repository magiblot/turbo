#include <turbo/clipboard.h>
#include <libclipboard.h>
#include <turbo/scintilla/internals.h>
#include <turbo/base64.h>

namespace turbo {

Clipboard::Clipboard() noexcept :
    selText(*new Scintilla::SelectionText)
{
}

Clipboard::~Clipboard()
{
    delete &selText;
}

void Clipboard::setText(TStringView text) noexcept
{
    selText.Copy(text, SC_CP_UTF8, SC_CHARSET_DEFAULT, false, true);
    xSetText(text);
}

void Clipboard::getText(TFuncView<void(TStringView)> accept) noexcept
{
    xGetText([&] (bool ok, TStringView text) {
        if (ok)
            selText.Copy(text, SC_CP_UTF8, SC_CHARSET_DEFAULT, false, true);
        accept({selText.Data(), selText.Length()});
    });
}

void Clipboard::setSelection(TFuncView<void(Scintilla::SelectionText &)> fill) noexcept
{
    fill(selText);
    xSetText({selText.Data(), selText.Length()});
}

void Clipboard::getSelection(TFuncView<void(const Scintilla::SelectionText &)> accept) noexcept
{
    xGetText([&] (bool ok, TStringView text) {
        if (ok)
            selText.Copy(text, SC_CP_UTF8, SC_CHARSET_DEFAULT, false, true);
        accept(selText);
    });
}

SystemClipboard::SystemClipboard() noexcept
{
    cb = clipboard_new(nullptr);
}

SystemClipboard::~SystemClipboard()
{
    clipboard_free(cb);
}

bool SystemClipboard::xSetText(TStringView text) noexcept
{
    #if !defined( _WIN32 )
    // Try to set clipboard via OSC 52 escape sequence on *nix platforms
    std::string encoded = turbo::to_base64(text.data());
    printf("\e]52;;%s\a", encoded.c_str());
    fflush(stdout);
    #endif

    return cb && clipboard_set_text_ex(cb, text.data(), (int) text.size(), LCB_CLIPBOARD);
}

void SystemClipboard::xGetText(TFuncView<void(bool, TStringView)> accept) noexcept
{
    char *text = cb ? clipboard_text(cb) : nullptr;
    accept(text != nullptr, text);
    ::free(text);
}

} // namespace turbo
