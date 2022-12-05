#include <turbo/clipboard.h>
#include <libclipboard.h>
#include <turbo/scintilla/internals.h>

// FIXME: We should not use Turbo Vision internals.
#include <tvision/internal/base64.h>

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
#ifndef _WIN32
    // FIXME: We should not access the terminal directly; this feature should
    // be implemented in Turbo Vision.
    ttyout = fopen("/dev/tty", "w");
#endif
}

SystemClipboard::~SystemClipboard()
{
    clipboard_free(cb);
#ifndef _WIN32
    fclose(ttyout);
#endif
}

static void putString(TStringView s, FILE *f) noexcept
{
    size_t written = 0, r;
    while (
        r = fwrite(s.data(), 1, s.size(), f),
        r != 0 && (written += r) < s.size()
    );
}

static void manipulateXTermSelectionData(TStringView text, FILE *out) noexcept
{
    TSpan<const uint8_t> data {(const uint8_t *) text.data(), text.size()};
    putString("\x1B]52;;", out); // OSC 52
    putString(tvision::encodeBase64(data), out);
    putString("\x07", out);
    fflush(out);
}

void SystemClipboard::xSetText(TStringView text) noexcept
{
    if (cb && clipboard_set_text_ex(cb, text.data(), (int) text.size(), LCB_CLIPBOARD))
        return;
#ifndef _WIN32
    if (ttyout)
        manipulateXTermSelectionData(text, ttyout);
#endif
}

void SystemClipboard::xGetText(TFuncView<void(bool, TStringView)> accept) noexcept
{
    char *text = cb ? clipboard_text(cb) : nullptr;
    accept(text != nullptr, text);
    ::free(text);
}

} // namespace turbo
