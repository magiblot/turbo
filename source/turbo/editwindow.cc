#define Uses_TScrollBar
#define Uses_TFrame
#define Uses_MsgBox
#define __INC_EDITORS_H
#include <tvision/tv.h>

#include "editwindow.h"
#include "editframe.h"
#include "app.h"
#include "apputils.h"
#include "search.h"
#include <fmt/core.h>
#include <iostream>
using std::ios;

TFrame *EditorWindow::initFrame(TRect bounds)
{
    return new EditorFrame(bounds);
}

EditorWindow::EditorWindow( const TRect &bounds, turbo::TScintilla &scintilla,
                            const char *filePath, active_counter &fileCounter,
                            EditorWindowParent &aParent ) noexcept :
    TWindowInit(&initFrame),
    TWindow(bounds, nullptr, wnNoNumber),
    editor(scintilla, filePath),
    listHead(this),
    fileNumber(fileCounter),
    parent(aParent)
{
    using namespace turbo;
    ((EditorFrame *) frame)->scintilla = &editor.scintilla;

    options |= ofTileable | ofFirstClick;
    setState(sfShadow, False);

    auto *editorView = new EditorView(TRect(1, 1, size.x - 1, size.y - 1));
    insert(editorView);

    auto *leftMargin = new LeftMarginView(leftMarginSep);
    leftMargin->options |= ofFramed;
    insert(leftMargin);

    auto *hScrollBar = new TScrollBar(TRect(18, size.y - 1, size.x - 2, size.y));
    hScrollBar->hide();
    insert(hScrollBar);

    auto *vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar->hide();
    insert(vScrollBar);

    editor.associate(this, editorView, leftMargin, hScrollBar, vScrollBar);

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
    editor.disassociate();
    TWindow::shutDown();
    parent.removeEditor(*this);
}

void EditorWindow::handleEvent(TEvent &ev) {
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
    TWindow::handleEvent(ev);
}


void EditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    auto lastResizeLock = editor.resizeLock;
    auto lastSize = size;
    editor.resizeLock = true;
    TWindow::dragView(event, mode, limits, minSize, maxSize);
    editor.resizeLock = lastResizeLock;
    if (lastSize != size)
        editor.redraw(); // Redraw without 'resizeLock = true'.
}

void EditorWindow::setState(ushort aState, Boolean enable)
{
    TWindow::setState(aState, enable);
    switch (aState)
    {
        case sfActive:
            updateCommands();
            if (editor.parent == this) // 'disassociate' not invoked yet.
            {
                editor.hScrollBar->setState(sfVisible, enable);
                editor.vScrollBar->setState(sfVisible, enable);
                if (enable)
                    parent.handleFocus(*this);
            }
            break;
    }
}

Boolean EditorWindow::valid(ushort command)
{
    if (TWindow::valid(command))
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

void EditorWindow::sizeLimits( TPoint& min, TPoint& max )
{
    TView::sizeLimits(min, max);
    min = minSize;
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

void EditorWindow::handleNotification(ushort code, turbo::Editor &) noexcept
{
    using namespace turbo;
    switch (code)
    {
        case Editor::ncPainted:
            if (!editor.resizeLock && frame) // These already get drawn when resizing.
                frame->drawView(); // The frame is sensible to the save point state.
            break;
        case FileEditor::ncSaved:
            updateCommands();
            parent.handleTitleChange(*this);
            editor.redraw();
            break;
    }
}

#define cpEditorWindow \
    "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97" \
    "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7"

TPalette& EditorWindow::getPalette() const
{
    static TPalette palette(cpEditorWindow, sizeof(cpEditorWindow) - 1);
    return palette;
}
