#define Uses_TScrollBar
#define Uses_TFrame
#define Uses_MsgBox
#define __INC_EDITORS_H
#include <tvision/tv.h>

#include "editwindow.h"
#include "editframe.h"
#include "docview.h"
#include "app.h"
#include "search.h"
#include "styles.h"
#include <fmt/core.h>
#include <iostream>
using std::ios;

TFrame *EditorWindow::initFrame(TRect bounds)
{
    return new EditorFrame(bounds);
}

EditorWindow::EditorWindow( const TRect &bounds, std::string_view aFile,
                            bool openCanFail ) :
    TWindowInit(&initFrame),
    TWindow(bounds, nullptr, wnNoNumber),
    drawing(false),
    resizeLock(false),
    lastSize(size),
    lineNumbers(5),
    editorView(editorSize(), theming),
    MRUhead(this),
    fatalError(false),
    inSavePoint(true)
{
    ((EditorFrame *) frame)->editwin = this;

    options |= ofTileable | ofFirstClick;
    setState(sfShadow, False);

    if (TurboApp::app)
        editor.clipboard = &TurboApp::app->clipboard;

    hScrollBar = new TScrollBar(TRect( 18, size.y - 1, size.x - 2, size.y ));
    hScrollBar->hide();
    insert(hScrollBar);

    vScrollBar = new TScrollBar(TRect( size.x - 1, 1, size.x, size.y - 1 ));
    vScrollBar->hide();
    insert(vScrollBar);

    leftMargin = new TSurfaceView(TRect( 1, 1, 1, size.y - 1 ), &editorView);
    leftMargin->options |= ofFramed;
    leftMargin->growMode = gfGrowHiY | gfFixed;
    insert(leftMargin);

    docView = new DocumentView( TRect( 1, 1, size.x - 1, size.y - 1 ),
                                &editorView,
                                editor,
                                *this );
    insert(docView);

    SearchBox::init(*this);

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

    setUpEditor(aFile, openCanFail);
}

EditorWindow::~EditorWindow()
{
    if (TurboApp::app)
        TurboApp::app->removeEditor(this);
}

TPoint EditorWindow::editorSize() const
{
    // Editor size: the window's inside.
    TRect r = getExtent().grow(-1, -1);
    if (lineNumbers.isEnabled())
        r.b.x--;
    return r.b - r.a;
}


TPoint EditorWindow::cursorPos()
{
    auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
    int line = std::min<size_t>(editor.WndProc(SCI_LINEFROMPOSITION, pos, 0U), INT_MAX);
    int col = std::min<size_t>(editor.WndProc(SCI_GETCOLUMN, pos, 0U), INT_MAX);
    return {col, line};
}

void EditorWindow::setUpEditor(std::string_view aFile, bool openCanFail)
{
    // Editor should take into account the size of docView.
    editor.setWindow(&editorView);
    // But should send notifications to this window.
    editor.setParent(this);
    // Set color defaults.
    theming.resetStyles(*this);
    // Open the current file, if set.
    tryLoadFile(aFile, openCanFail);
    // Apply the properties detected while loading the file.
    props.apply(editor);

    // Dynamic horizontal scroll
    editor.WndProc(SCI_SETSCROLLWIDTHTRACKING, true, 0U);
    editor.WndProc(SCI_SETSCROLLWIDTH, 1, 0U);
    editor.WndProc(SCI_SETXCARETPOLICY, CARET_EVEN, 0);
    // Trick so that the scroll width gets computed.
    editor.WndProc(SCI_SETFIRSTVISIBLELINE, 1, 0U);
    editor.WndProc(SCI_SETFIRSTVISIBLELINE, 0, 0U);
    editor.draw(editorView);

    // Enable line wrapping (if appropiate) by default
    wrap.toggle(editor, false);
    editor.WndProc(SCI_SETWRAPVISUALFLAGS, SC_WRAPVISUALFLAG_END, 0U);

    // Home/End keys should respect line wrapping.
    editor.WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_NORM << 16), SCI_VCHOMEWRAP);
    editor.WndProc(SCI_ASSIGNCMDKEY, SCK_HOME | (SCI_SHIFT << 16), SCI_VCHOMEWRAPEXTEND);
    editor.WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_NORM << 16), SCI_LINEENDWRAP);
    editor.WndProc(SCI_ASSIGNCMDKEY, SCK_END | (SCI_SHIFT << 16), SCI_LINEENDWRAPEXTEND);

    // Clear the undo buffer created when loading the file,
    // if that's the case.
    editor.WndProc(SCI_EMPTYUNDOBUFFER, 0U, 0U);
    inSavePoint = true;

    // Indentation
    editor.WndProc(SCI_SETUSETABS, false, 0U);
    editor.WndProc(SCI_SETINDENT, 4, 0U);
    editor.WndProc(SCI_SETTABINDENTS, true, 0U);
    editor.WndProc(SCI_SETBACKSPACEUNINDENTS, true, 0U);

    // Line numbers
    editor.WndProc(SCI_SETMARGINS, 1, 0U);
    editor.WndProc(SCI_SETMARGINTYPEN, 0, SC_MARGIN_NUMBER);
    updateMarginWidth();

    redrawEditor();
}

void EditorWindow::redrawEditor()
{
    if (!drawing) {
        drawing = true;
        lock();
        updateMarginWidth();
        if (!resizeLock)
        {
            editor.changeSize();
            theming.updateBraces(editor);
        }
        editor.draw(editorView);
        leftMargin->drawView();
        docView->drawView();
        hScrollBar->drawView();
        vScrollBar->drawView();
        frame->drawView();
        unlock();
        drawing = false;
    }
}

void EditorWindow::updateMarginWidth()
{
    const auto [width, delta] = lineNumbers.update(editor);
    if (delta) {
        editor.WndProc(SCI_SETMARGINWIDTHN, 0, width);
        {
            TRect r = leftMargin->getBounds();
            r.b.x += delta;
            leftMargin->changeBounds(r);
        }
        {
            TRect dv = docView->getBounds();
            dv.a.x += delta;
            TPoint ev = editorView.size;
            if (width == 0) { // Line numbers disabled.
                dv.a.x -= 1;
                ev.x += 1;
            } else if (width == delta) { // Line numbers enabled.
                dv.a.x += 1;
                ev.x -= 1;
            }
            docView->changeBounds(dv);
            docView->delta = {width, 0};
            editorView.resize(ev);
        }
        frame->drawView();
    }
}

void EditorWindow::handleEvent(TEvent &ev) {
    if (ev.what == evBroadcast && ev.message.command == cmScrollBarChanged) {
        if (scrollBarChanged((TScrollBar *) ev.message.infoPtr)) {
            redrawEditor();
            clearEvent(ev);
        }
    }
    if (ev.what == evCommand) {
        bool handled = true;
        switch (ev.message.command) {
            case cmSave:
                trySaveFile();
                break;
            case cmSaveAs:
                saveAsDialog();
                break;
            case cmRename:
                renameDialog();
                break;
            case cmToggleWrap:
                if (wrap.toggle(editor))
                    redrawEditor();
                break;
            case cmToggleLineNums:
                lineNumbers.toggle();
                redrawEditor();
                break;
            case cmToggleIndent:
                indent.toggle();
                break;
            case cmCloseEditor: {
                TEvent event = ev;
                event.message.command = cmClose;
                TWindow::handleEvent(event);
                handled = (event.what == evNothing);
            }
            default:
                handled = false;
        }
        if (handled)
            clearEvent(ev);
    }
    TWindow::handleEvent(ev);
}

void EditorWindow::changeBounds(const TRect &bounds)
{
    lock();
    lockSubViews();
    TWindow::changeBounds(bounds);
    unlockSubViews();
    if (size != lastSize) {
        editorView.grow(size - lastSize);
        redrawEditor();
        lastSize = size;
    }
    unlock();
}

void EditorWindow::dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize)
{
    resizeLock = true;
    TWindow::dragView(event, mode, limits, minSize, maxSize);
    resizeLock = false;
    redrawEditor();
}

void EditorWindow::setState(ushort aState, Boolean enable)
{
    TWindow::setState(aState, enable);
    if (state & sfExposed) {
        // Otherwise, we could be in the middle of shutdown and our
        // subviews could have already been free'd.
        switch (aState) {
            case sfActive:
                hScrollBar->setState(sfVisible, enable);
                vScrollBar->setState(sfVisible, enable);
                if (enable && TurboApp::app)
                    TurboApp::app->setFocusedEditor(this);
                break;
        }
    }
    switch (aState) {
        // These actions do not depend on subview lifetime.
        case sfActive:
            updateCommands();
            break;
    }

}

Boolean EditorWindow::valid(ushort command)
{
    if (TWindow::valid(command)) {
        switch (command) {
            case cmValid:
                return Boolean(!fatalError);
            case cmQuit:
                return Boolean(tryClose());
        }
        return True;
    }
    return False;
}

const char* EditorWindow::getTitle(short)
{
    if (!title.empty())
        return title.c_str();
    if (!file.empty())
        return file.c_str();
    return nullptr;
}

void EditorWindow::sizeLimits( TPoint& min, TPoint& max )
{
    TView::sizeLimits(min, max);
    min = minEditWinSize;
}

void EditorWindow::updateCommands()
{
    if (state & sfActive)
        enableCommands(enabledCmds);
    else
        disableCommands(disabledCmds);
}

void EditorWindow::lockSubViews()
{
    for (auto *v : std::initializer_list<TView *> {docView, leftMargin, vScrollBar})
        v->setState(sfExposed, False);
}

void EditorWindow::unlockSubViews()
{
    for (auto *v : std::initializer_list<TView *> {docView, leftMargin, vScrollBar})
        v->setState(sfExposed, True);
}

void EditorWindow::scrollBarEvent(TEvent ev)
{
    hScrollBar->handleEvent(ev);
    vScrollBar->handleEvent(ev);
}

bool EditorWindow::scrollBarChanged(TScrollBar *bar)
{
    if (bar == vScrollBar) {
        editor.WndProc(SCI_SETFIRSTVISIBLELINE, bar->value, 0U);
        return true;
    } else if (bar == hScrollBar) {
        editor.WndProc(SCI_SETXOFFSET, bar->value, 0U);
        return true;
    }
    return false;
}

void EditorWindow::scrollTo(TPoint delta)
{
    {
        auto lk {lockDrawing()};
        hScrollBar->setValue(delta.x);
        vScrollBar->setValue(delta.y);
    }
    redrawEditor();
}

void EditorWindow::notify(SCNotification scn)
{
    switch (scn.nmhdr.code) {
        case SCN_SAVEPOINTLEFT: setSavePointLeft(); break;
        case SCN_SAVEPOINTREACHED: setSavePointReached(); break;
        case SCN_CHARADDED:
            if (scn.ch == '\n')
                indent.autoIndentCurrentLine(editor);
            break;
    }
}

void EditorWindow::setHorizontalScrollPos(int delta, int limit)
{
    int size = docView->size.x;
    hScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
}

void EditorWindow::setVerticalScrollPos(int delta, int limit)
{
    int size = docView->size.y;
    vScrollBar->setParams(delta, 0, limit - size, size - 1, 1);
}

/////////////////////////////////////////////////////////////////////////
// Save point and window title state.

void EditorWindow::setSavePointLeft()
{
    using namespace std::literals;
    if (inSavePoint) {
        inSavePoint = false;
        title.append("*"sv);
        frame->drawView();
    }
}

void EditorWindow::setSavePointReached()
{
    if (!inSavePoint) {
        inSavePoint = true;
        title.erase(title.size() - 1, 1);
        frame->drawView();
    }
}

void EditorWindow::setSavePoint()
{
    setSavePointReached();
    editor.WndProc(SCI_SETSAVEPOINT, 0U, 0U);
    frame->drawView();
}

/////////////////////////////////////////////////////////////////////////
// File opening and saving.

#include <memory>
#include <fstream>
#include <cstdio>
#include <cerrno>

void EditorWindow::setFile(std::string newFile)
{
    if (!newFile.empty()) {
        enabledCmds += cmRename;
        updateCommands();
    }
    std::string oldFile = std::move(file);
    file = std::move(newFile);
    if (TurboApp::app)
        TurboApp::app->updateEditorTitle(this, oldFile);
    theming.detectLanguage(*this);
}

// Note: the 'fatalError' variable set here is later checked in valid() for
// command cmValid. If there was an error, valid() will return False,
// thus resulting in the EditorWindow being destroyed in checkValid().

void EditorWindow::tryLoadFile(std::string_view aFile, bool canFail)
{
    if (!aFile.empty()) {
        char fileName[MAXPATH];
        TPath::resolve(fileName, aFile);
        fatalError = !loadFile(fileName, canFail);
        if (!fatalError)
            setFile(fileName);
    }
}

bool EditorWindow::loadFile(const char *file, bool canFail)
{
    std::ifstream f(file, ios::in | ios::binary);
    if (f) {
        f.seekg(0, ios::end);
        size_t fSize = f.tellg();
        f.seekg(0);
        // Allocate 1000 extra bytes, as in SciTE.
        editor.WndProc(SCI_ALLOCATE, fSize + 1000, 0U);
        // Get ready to detect the document's properties.
        props.reset();
        if (fSize) {
            bool ok = true;
            constexpr size_t blockSize = 1 << 20; // Read in chunks of 1 MiB.
            size_t readSize = std::min(fSize, blockSize);
            auto buffer = std::make_unique<char[]>(readSize);
            sptr_t wParam = reinterpret_cast<sptr_t>(buffer.get());
            while (fSize > 0 && (ok = bool(f.read(buffer.get(), readSize)))) {
                props.analyze({buffer.get(), readSize});
                editor.WndProc(SCI_APPENDTEXT, readSize, wParam);
                fSize -= readSize;
                if (fSize < readSize)
                    readSize = fSize;
            };
            if (!ok) {
                showError(fmt::format("An error occurred while reading from file '{}': {}.", file, strerror(errno)));
                return false;
            }
        }
    } else if (!canFail) {
        showError(fmt::format("Unable to open file '{}': {}.", file, strerror(errno)));
        return false;
    }
    return true;
}

bool EditorWindow::trySaveFile()
{
    if (!inSavePoint || file.empty()) {
        if (file.empty())
            return saveAsDialog();
        return saveFile(file.c_str());
    }
    return true;
}

void EditorWindow::processBeforeSave()
{
    // Don't modify the file if it is still clean.
    if (!inSavePoint) {
        ::stripTrailingSpaces(editor);
        ::ensureNewlineAtEnd(editor, props.getEOLType());
        redrawEditor();
    }
}

void EditorWindow::processAfterSave()
{
    setSavePoint();
}

bool EditorWindow::saveFile(const char *file, bool silent)
{
    processBeforeSave();
    std::ofstream f(file, ios::out | ios::binary);
    if (f) {
        size_t bytesLeft = editor.WndProc(SCI_GETTEXT, 0, 0) - 1;
        if (bytesLeft) {
            bool ok = true;
            constexpr size_t blockSize = 1 << 20; // Write in chunks of 1 MiB.
            size_t writeSize = std::min(bytesLeft, blockSize);
            auto buffer = std::make_unique<char[]>(writeSize);
            sptr_t bufParam = reinterpret_cast<sptr_t>(buffer.get());
            size_t i = 0;
            do {
                editor.WndProc(SCI_SETTARGETRANGE, i, i + writeSize);
                editor.WndProc(SCI_GETTARGETTEXT, 0U, bufParam);
                ok = (bool) f.write(buffer.get(), writeSize);
                i += writeSize;
                bytesLeft -= writeSize;
                if (bytesLeft < writeSize)
                    writeSize = bytesLeft;
            } while (bytesLeft > 0 && ok);
            if (!ok) {
                if (!silent) showError(fmt::format("An error occurred while writing to file '{}': {}.", file, strerror(errno)));
                return false;
            }
        }
    } else {
        if (!silent) showError(fmt::format("Unable to write into file '{}': {}.", file, strerror(errno)));
        return false;
    }
    processAfterSave();
    return true;
}

bool EditorWindow::saveAsDialog()
{
    if (TurboApp::app) {
        bool saved = false;
        TurboApp::app->openFileDialog("*.*", "Save file as", "~N~ame", fdOKButton, 0,
            [this, &saved] (TView *dialog) {
                char fileName[MAXPATH];
                dialog->getData(fileName);
                fexpand(fileName);
                if (canOverwrite(fileName) && saveFile(fileName)) {
                    setFile(fileName);
                    return ((saved = true));
                }
                return false;
            }
        );
        return saved;
    }
    return false;
}

void EditorWindow::renameDialog()
{
    if (TurboApp::app) {
        TurboApp::app->openFileDialog("*.*", "Rename file", "~N~ame", fdOKButton, 0,
            [this] (TView *dialog) {
                char newFile[MAXPATH];
                dialog->getData(newFile);
                fexpand(newFile);
                // Don't do anything if renaming to the same file. If the user needed to
                // save the file, they would use the 'save' feature.
                if (std::string_view {newFile} == file)
                    return true;
                if (canOverwrite(newFile) && renameFile(file.c_str(), newFile)) {
                    setFile(newFile);
                    return true;
                }
                return false;
            }
        );
    }
}

bool EditorWindow::renameFile(const char *src, const char *dst)
{
    // Try saving first, then renaming.
    if (saveFile(src, true) && ::rename(src, dst) == 0)
        return true;
    // If the above doesn't work, try saving at the new location, and then remove
    // the old file.
    else if (saveFile(dst, true)) {
        if (TPath::exists(src) && ::remove(src) != 0)
            showWarning(fmt::format("'{}' was created successfully, but '{}' could not be removed: {}.", dst, src, strerror(errno)));
        return true;
    }
    showError(fmt::format("Unable to rename '{}' into '{}': {}.", src, dst, strerror(errno)));
    return false;
}

bool EditorWindow::canOverwrite(const char *file)
{
    if (TPath::exists(file)) {
        auto &&text = fmt::format("'{}' already exists. Overwrite?", file);
        return cmYes == messageBox(text, mfConfirmation | mfYesButton | mfNoButton);
    }
    return true;
}

bool EditorWindow::tryClose()
{
    if (!inSavePoint) {
        auto &&msg = file.empty() ? fmt::format("Save '{}'?", name)
                                  : fmt::format("'{}' has been modified. Save?", file);
        switch (messageBox(msg, mfConfirmation | mfYesNoCancel)) {
            case cmYes:
                return trySaveFile(); // Close only if the file gets saved.
            case cmNo:
                return true;
            default:
                return false;
        }
    }
    return true;
}

void EditorWindow::close()
{
    if (tryClose())
        TWindow::close();
}

void EditorWindow::showError(std::string_view s)
{
    messageBox(s, mfError | mfOKButton);
}

void EditorWindow::showWarning(std::string_view s)
{
    messageBox(s, mfWarning | mfOKButton);
}

#define cpEditorWindow \
    "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97" \
    "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7"

TPalette& EditorWindow::getPalette() const
{
    static TPalette palette(cpEditorWindow, sizeof(cpEditorWindow) - 1);
    return palette;
}
