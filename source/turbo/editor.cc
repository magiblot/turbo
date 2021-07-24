#include "editor.h"
#include "editwindow.h"

void TurboEditorState::afterSave() noexcept
{
    using namespace turbo;
    auto lastLang = theming.language;
    super::afterSave();
    if (lastLang == langNone && theming.language != langNone)
        lineNumbers.enabled = true;
}

void TurboFileDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getOpenPath(accept);
}

void TurboFileDialogs::getSaveAsPath(turbo::FileEditorState &state, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getSaveAsPath(state, accept);
}

void TurboFileDialogs::getRenamePath(turbo::FileEditorState &state, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getRenamePath(state, accept);
}
