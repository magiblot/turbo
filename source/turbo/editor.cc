#include "editor.h"
#include "editwindow.h"

void TurboEditor::afterSave() noexcept
{
    using namespace turbo;
    bool noLastLexer = !theming.lexInfo;
    super::afterSave();
    if (noLastLexer && theming.lexInfo)
        lineNumbers.enabled = true;
}

void TurboFileDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getOpenPath(accept);
}

void TurboFileDialogs::getSaveAsPath(turbo::FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getSaveAsPath(editor, accept);
}

void TurboFileDialogs::getRenamePath(turbo::FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    CwdGuard cwd {app.getFileDialogDir()};
    super::getRenamePath(editor, accept);
}
