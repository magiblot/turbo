#include "editor.h"
#include "editwindow.h"

void TurboEditor::onFilePathSet() noexcept
{
    bool noLastLexer = !theming.hasLexer();
    super::onFilePathSet();
    if (noLastLexer && theming.hasLexer())
        lineNumbers.setState(true);
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
