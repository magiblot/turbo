#ifndef TURBO_EDITOR_H
#define TURBO_EDITOR_H

#include <turbo/turbo.h>

struct EditorWindowParent;

class TurboFileDialogs : public turbo::DefaultFileDialogs
{
    using super = turbo::DefaultFileDialogs;

    EditorWindowParent &app;

    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(turbo::FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(turbo::FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;

public:

    TurboFileDialogs(EditorWindowParent &aApp) :
        app(aApp)
    {
    }
};

class TurboEditor : public turbo::FileEditor
{
    using super = turbo::FileEditor;

public:

    template <class... Args>
    TurboEditor(Args&&... args) noexcept :
        super(static_cast<Args&&>(args)...)
    {
        if (theming.language != turbo::langNone)
            lineNumbers.enabled = true;
        wrapping.toggle(scintilla, [] (int) { return false; });
    }

    void afterSave() noexcept override;
};

#endif // TURBO_EDITOR_H
