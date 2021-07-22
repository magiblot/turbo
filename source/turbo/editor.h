#ifndef TURBO_EDITOR_H
#define TURBO_EDITOR_H

#include <turbo/turbo.h>

struct EditorWindowParent;

class TurboFileDialogs : public turbo::DefaultFileDialogs
{
    using super = turbo::DefaultFileDialogs;

    EditorWindowParent &app;

    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(turbo::FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(turbo::FileEditorState &, TFuncView<bool (const char *)> accept) noexcept override;

public:

    TurboFileDialogs(EditorWindowParent &aApp) :
        app(aApp)
    {
    }
};

class TurboEditorState : public turbo::FileEditorState
{
    using super = turbo::FileEditorState;

public:

    template <class... Args>
    TurboEditorState(Args&&... args) noexcept :
        super(static_cast<Args&&>(args)...)
    {
        if (theming.language != turbo::langNone)
            lineNumbers.enabled = true;
    }

    void afterSave() noexcept override;
};

#endif // TURBO_EDITOR_H
