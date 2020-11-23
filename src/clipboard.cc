#include "clipboard.h"
#include <libclipboard.h>

Clipboard::Clipboard()
{
    cb = clipboard_new(nullptr);
}

Clipboard::~Clipboard()
{
    clipboard_free(cb);
}

void Clipboard::syncSetText(std::string_view text)
{
    if (cb)
        clipboard_set_text_ex(cb, text.data(), (int) text.size(), LCB_CLIPBOARD);
}

const char* Clipboard::syncGetText()
{
    if (cb)
        return clipboard_text(cb);
    return nullptr;
}
