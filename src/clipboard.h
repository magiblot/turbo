#ifndef TURBO_CLIPBOARD_H
#define TURBO_CLIPBOARD_H

#include <tvision/tv.h>
#include <turbo/funcview.h>
#include <ScintillaHeaders.h>

using turbo::FuncView;

class Clipboard
{
    Scintilla::SelectionText selText;

public:

    virtual void xSetText(TStringView) noexcept = 0;
    virtual void xGetText(FuncView<void(bool, TStringView)> accept) noexcept = 0;

    template <class Func>
    void copy(Func &&fillSel) noexcept;
    template <class Func>
    void paste(Func &&fillSel) noexcept;
};

template <class Func>
inline void Clipboard::copy(Func &&fillSel) noexcept
{
    fillSel(selText);
    xSetText({selText.Data(), selText.Length()});
}

template <class Func>
inline void Clipboard::paste(Func &&fillSel) noexcept
{
    xGetText([&] (bool ok, TStringView text) {
        fillSel(
            selText,
            ok ? text : TStringView {selText.Data(), selText.Length()}
        );
    });
}

class LcbClipboard : public Clipboard
{
    struct clipboard_c *cb;

public:

    LcbClipboard() noexcept;
    ~LcbClipboard() noexcept;

    LcbClipboard(const LcbClipboard &) = delete;
    LcbClipboard& operator=(const LcbClipboard &) = delete;

    void xSetText(TStringView) noexcept override;
    void xGetText(FuncView<void(bool, TStringView)> accept) noexcept override;

};

#endif // TURBO_CLIPBOARD_H
