#define Uses_TScrollBar
#define Uses_TFrame
#define Uses_MsgBox
#define Uses_TKeys
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
    searchBox = &SearchBox::create(getExtent().grow(-1, -1), editor);
    insert(searchBox);

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
    enabledCmds += cmSelUppercase;
    enabledCmds += cmSelLowercase;
    enabledCmds += cmSelCapitalize;
    enabledCmds += cmToggleComment;

    // Commands that always get disabled when unfocusing the editor.
    disabledCmds += enabledCmds;
    disabledCmds += cmRename;
}

void EditorWindow::shutDown()
{
    parent.removeEditor(*this);
    searchBox = nullptr;
    super::shutDown();
}

void EditorWindow::handleEvent(TEvent &ev)
{
    auto &editor = getEditor();
    TurboFileDialogs dlgs {parent};
    bool handled = true;
    switch (ev.what)
    {
        case evKeyDown:
            switch (ev.keyDown.keyCode)
            {
                case kbEsc:
                    handled = (searchBox && searchBox->close());
                    break;
                default:
                    handled = false;
            }
            break;
        case evCommand:
            switch (ev.message.command)
            {
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
                    editor.wrapping.toggle(editor.scintilla);
                    editor.redraw();
                    break;
                case cmToggleLineNums:
                    editor.lineNumbers.toggle();
                    editor.redraw();
                    break;
                case cmToggleIndent:
                    editor.autoIndent.toggle();
                    break;
                case cmCloseEditor:
                    ev.message.command = cmClose;
                    handled = false;
                    break;
                case cmSelUppercase:
                    editor.uppercase();
                    editor.partialRedraw();
                    break;
                case cmSelLowercase:
                    editor.lowercase();
                    editor.partialRedraw();
                    break;
                case cmSelCapitalize:
                    editor.capitalize();
                    editor.partialRedraw();
                    break;
                case cmToggleComment:
                    editor.toggleComment();
                    editor.partialRedraw();
                    break;
                case cmFind:
                case cmReplace:
                    if (searchBox)
                        searchBox->open();
                    break;
                default:
                    handled = false;
            }
        default:
            handled = false;
    }
    if (handled)
        clearEvent(ev);
    else
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
    return formatTitle();
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

void EditorWindow::handleNotification(const SCNotification &scn, turbo::Editor &editor)
{
    using namespace turbo;
    super::handleNotification(scn, editor);
    switch (scn.nmhdr.code)
    {
        case SCN_SAVEPOINTREACHED:
            updateCommands();
            parent.handleTitleChange(*this);
            editor.redraw();
            break;
    }
}

const char* EditorWindow::formatTitle(ushort flags) noexcept
{
    bool inSavePoint = (flags & tfNoSavePoint) || editor.inSavePoint();
    TitleState titleState {fileNumber.counter, fileNumber.number, inSavePoint};
    if (lastTitleState != titleState)
    {
        lastTitleState = titleState;
        TStringView name = !filePath().empty() ? TPath::basename(filePath()) : "Untitled";
        auto &&number = fileNumber.number > 1 ? fmt::format(" ({})", fileNumber.number) : std::string();
        TStringView savePoint = inSavePoint ? "" : "*";
        title = fmt::format("{}{}{}", name, number, savePoint);
    }
    return title.c_str();
}
