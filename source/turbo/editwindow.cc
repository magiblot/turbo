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

EditorWindow::EditorWindow( const TRect &bounds, turbo::Scintilla &scintilla,
                            const char *filePath, active_counter &fileCounter,
                            EditorWindowParent &aParent ) noexcept :
    TWindowInit(&initFrame),
    TWindow(bounds, nullptr, wnNoNumber),
    editorState(scintilla, filePath),
    listHead(this),
    fileNumber(fileCounter),
    parent(aParent)
{
    using namespace turbo;
    ((EditorFrame *) frame)->scintilla = &editorState.scintilla;

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

    editorState.associate(this, editorView, leftMargin, hScrollBar, vScrollBar);

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
    editorState.disassociate();
    TWindow::shutDown();
    parent.removeEditor(*this);
}

void EditorWindow::handleEvent(TEvent &ev) {
    if (ev.what == evCommand) {
        bool handled = true;
        TurboFileDialogs dlgs {parent};
        switch (ev.message.command) {
            case cmSave:
                editorState.save(dlgs);
                break;
            case cmSaveAs:
                editorState.saveAs(dlgs);
                break;
            case cmRename:
                editorState.rename(dlgs);
                break;
            case cmToggleWrap:
                editorState.toggleLineWrapping();
                editorState.redraw();
                break;
            case cmToggleLineNums:
                editorState.toggleLineNumbers();
                editorState.redraw();
                break;
            case cmToggleIndent:
                editorState.toggleAutoIndent();
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
    auto lastResizeLock = editorState.resizeLock;
    auto lastSize = size;
    editorState.resizeLock = true;
    TWindow::dragView(event, mode, limits, minSize, maxSize);
    editorState.resizeLock = lastResizeLock;
    if (lastSize != size)
        editorState.redraw(); // Redraw without 'resizeLock = true'.
}

void EditorWindow::setState(ushort aState, Boolean enable)
{
    TWindow::setState(aState, enable);
    switch (aState)
    {
        case sfActive:
            updateCommands();
            if (editorState.parent == this) // 'disassociate' not invoked yet.
            {
                editorState.hScrollBar->setState(sfVisible, enable);
                editorState.vScrollBar->setState(sfVisible, enable);
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
            return editorState.close(dlgs);
        }
        return true;
    }
    return false;
}

const char* EditorWindow::getTitle(short)
{
    TitleState titleState {fileNumber.counter, fileNumber.number, editorState.inSavePoint()};
    if (lastTitleState != titleState)
    {
        lastTitleState = titleState;
        TStringView name = !filePath().empty() ? TPath::basename(filePath()) : "Untitled";
        TStringView savePoint = editorState.inSavePoint() ? "" : "*";
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

void EditorWindow::handleNotification(ushort code, turbo::EditorState &) noexcept
{
    using namespace turbo;
    switch (code)
    {
        case EditorState::ncPainted:
            if (!editorState.resizeLock && frame) // These already get drawn when resizing.
                frame->drawView(); // The frame is sensible to the save point state.
            break;
        case FileEditorState::ncSaved:
            updateCommands();
            parent.handleTitleChange(*this);
            editorState.redraw();
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
