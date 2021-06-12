#ifndef TURBO_CLIPBOARD_H
#define TURBO_CLIPBOARD_H

#include <ScintillaHeaders.h>
#include <string_view>
#include <cstdlib>

class Clipboard {

    Scintilla::SelectionText selText;
    struct clipboard_c *cb;

    void syncSetText(std::string_view);
    const char* syncGetText();

protected:

    virtual void fallbackSetText(std::string_view);
    virtual char* fallbackGetText();

public:

    Clipboard();
    virtual ~Clipboard();

    template <class Func>
    void copy(Func &&fillSel)
    {
        fillSel(selText);
        syncSetText({selText.Data(), selText.Length()});
    }

    template <class Func>
    void paste(Func &&fillSel)
    {
        auto *data = syncGetText();
        std::string_view text;
        if (data)
            text = data;
        else
            text = {selText.Data(), selText.Length()};
        fillSel(selText, text);
        ::free((void *) data);
    }

};

#endif // TURBO_CLIPBOARD_H
