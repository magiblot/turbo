#define Uses_TScrollBar
#define Uses_TFrame
#define Uses_MsgBox
#define __INC_EDITORS_H
#include <tvision/tv.h>

#include <turbo/tpath.h>
#include "editwindow.h"
#include "app.h"
#include "apputils.h"
#include "search.h"
#include <fmt/core.h>
#include <iostream>
using std::ios;

EditorWindow::EditorWindow( const TRect &bounds, TurboEditor &aEditor,
                            active_counter &fileCounter,
                            EditorWindowParent &aParent ) noexcept :
    TWindowInit(&initFrame),
    super(bounds, aEditor),
    listHead(this),
    fileNumber(fileCounter),
    parent(aParent)
{
    insertSearchBox(*this);

    // Commands that always get enabled when focusing the editor.
    enabledCmds += cmSave;
    enabledCmds += cmSaveAs;
    enabledCmds += cmToggleWrap;
    enabledCmds += cmToggleLineNums;
    enabledCmds += cmFind;
    enabledCmds += cmSearchAgain;
    enabledCmds += cmSearchPrev;
    enabledCmds += cmToggleIndent;
    enabledCmds += cmCloseEditor;

    // Commands that always get disabled when unfocusing the editor.
    disabledCmds += enabledCmds;
    disabledCmds += cmRename;
}

void EditorWindow::shutDown()
{
    super::shutDown();
    parent.removeEditor(*this);
}

void EditorWindow::handleEvent(TEvent &ev) {
    auto &editor = getEditor();
    if (ev.what == evCommand) {
        bool handled = true;
        TurboFileDialogs dlgs {parent};
        switch (ev.message.command) {
            case cmSave:
                editor.save(dlgs);
                break;
            case cmSaveAs:
                editor.saveAs(dlgs);
                break;
            case cmRename:
                editor.rename(dlgs);
                break;
            case cmToggleWrap:
                editor.toggleLineWrapping();
                editor.redraw();
                break;
            case cmToggleLineNums:
                editor.toggleLineNumbers();
                editor.redraw();
                break;
            case cmToggleIndent:
                editor.toggleAutoIndent();
                break;
            case cmCloseEditor:
                handled = message(this, evCommand, cmClose, nullptr); // May delete 'this'!
                break;
            default:
                handled = false;
        }
        if (handled)
        {
            clearEvent(ev);
            return;
        }
    }
    super::handleEvent(ev);
}

void EditorWindow::setState(ushort aState, Boolean enable)
{
    super::setState(aState, enable);
    if (aState == sfActive)
    {
        updateCommands();
        if (enable)
            parent.handleFocus(*this);
    }
}

Boolean EditorWindow::valid(ushort command)
{
    auto &editor = getEditor();
    if (super::valid(command))
    {
        if (command != cmValid)
        {
            TurboFileDialogs dlgs {parent};
            return editor.close(dlgs);
        }
        return true;
    }
    return false;
}

const char* EditorWindow::getTitle(short)
{
    TitleState titleState {fileNumber.counter, fileNumber.number, editor.inSavePoint()};
    if (lastTitleState != titleState)
    {
        lastTitleState = titleState;
        TStringView name = !filePath().empty() ? TPath::basename(filePath()) : "Untitled";
        TStringView savePoint = editor.inSavePoint() ? "" : "*";
        auto &&number = fileNumber.number > 1 ? fmt::format(" ({})", fileNumber.number) : std::string();
        title = fmt::format("{}{}{}", name, savePoint, number);
    }
    return title.c_str();
}

void EditorWindow::updateCommands() noexcept
{
    if (!filePath().empty())
        enabledCmds += cmRename;
    if (state & sfActive)
        enableCommands(enabledCmds);
    else
        disableCommands(disabledCmds);
}

void EditorWindow::handleNotification(ushort code, turbo::Editor &editor) noexcept
{
    using namespace turbo;
    super::handleNotification(code, editor);
    if (code == FileEditor::ncSaved)
    {
        updateCommands();
        parent.handleTitleChange(*this);
        editor.redraw();
    }
}
