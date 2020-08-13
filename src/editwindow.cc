#define Uses_TScrollBar
#define Uses_MsgBox
#define Uses_TIndicator
#include <tvision/tv.h>

#include "editwindow.h"
#include "docview.h"
#include "app.h"
#include "search.h"
#include "styles.h"
#include <fmt/core.h>

EditorWindow::EditorWindow( const TRect &bounds, std::string_view aFile,
                            bool openCanFail ) :
    TWindow(bounds, nullptr, wnNoNumber),
    TWindowInit(&initFrame),
    drawing(false),
    resizeLock(false),
    lastSize(size),
    lineNumbers(5),
    MRUhead(this),
    fatalError(false),
    file(aFile),
    inSavePoint(true),
    editorView(editorBounds())
{
    options |= ofTileable | ofFirstClick;
    setState(sfShadow, False);

    if (TurboApp::app)
        editor.clipboard = &TurboApp::app->clipboard;

    editorView.hide();

    hScrollBar = new TScrollBar(TRect( 18, size.y - 1, size.x - 2, size.y ));
    hScrollBar->hide();
    insert(hScrollBar);

    vScrollBar = new TScrollBar(TRect( size.x - 1, 1, size.x, size.y - 1 ));
    vScrollBar->hide();
    insert(vScrollBar);

    indicator = new TIndicator(TRect( 2, size.y - 1, 18, size.y ));
    indicator->hide();
    insert(indicator);

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

    // Set the commands that always get enabled when focusing the editor.
    commandSet += cmSave;
    commandSet += cmSaveAs;
    commandSet += cmToggleWrap;
    commandSet += cmToggleLineNums;
    commandSet += cmFind;
//     commandSet += cmReplace;
    commandSet += cmSearchAgain;
    commandSet += cmSearchPrev;
    commandSet += cmToggleIndent;

    setUpEditor(openCanFail);
}

EditorWindow::~EditorWindow()
{
    if (TurboApp::app)
        TurboApp::app->removeEditor(this);
}

TRect EditorWindow::editorBounds() const
{
    // Editor size: the window's inside.
    TRect r = getExtent().grow(-1, -1);
    if (lineNumbers.isEnabled())
        r.b.x--;
    return r;
}

void EditorWindow::setUpEditor(bool openCanFail)
{
    // Editor should take into account the size of docView.
    editor.setWindow(&editorView);
    // But should send notifications to this window.
    editor.setParent(this);
    // Set color defaults.
    setUpStyles(*this);
    // Open the current file, if set.
    tryLoadFile(openCanFail);
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
            editor.changeSize();
        editor.draw(editorView);
        leftMargin->drawView();
        docView->drawView();
        hScrollBar->drawView();
        vScrollBar->drawView();
        updateIndicatorValue();
        indicator->drawView();
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
            TRect ev = editorView.getBounds();
            if (width == 0) { // Line numbers disabled.
                dv.a.x -= 1;
                ev.b.x += 1;
            } else if (width == delta) { // Line numbers enabled.
                dv.a.x += 1;
                ev.b.x -= 1;
            }
            docView->changeBounds(dv);
            docView->setDelta({width, 0});
            editorView.changeBounds(ev);
        }
        frame->drawView();
    }
}

void EditorWindow::updateIndicatorValue()
{
    auto pos = editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
    int line = std::min<size_t>(editor.WndProc(SCI_LINEFROMPOSITION, pos, 0U), INT_MAX);
    int col = std::min<size_t>(editor.WndProc(SCI_GETCOLUMN, pos, 0U), INT_MAX);
    indicator->setValue({col, line}, False);
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
        editorView.changeBounds(editorBounds());
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
                indicator->setState(sfVisible, enable);
                if (enable && TurboApp::app)
                    TurboApp::app->setFocusedEditor(this);
                break;
        }
    }
    switch (aState) {
        // These actions do not depend on subview lifetime.
        case sfActive:
            (enable ? enableCommands
                    : disableCommands)(commandSet);
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
    editor.WndProc(SCI_SETXOFFSET, std::clamp(delta.x, 0, hScrollBar->maxVal), 0U);
    editor.WndProc(SCI_SETFIRSTVISIBLELINE, delta.y, 0U);
}

void EditorWindow::notify(SCNotification scn)
{
    switch (scn.nmhdr.code) {
        case SCN_SAVEPOINTLEFT: setSavePointLeft(); break;
        case SCN_SAVEPOINTREACHED: setSavePointReached(); break;
        case SCN_CHARADDED:
            if (scn.ch == '\n')
                indent.autoIndentCurrentLine(editor);
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
    inSavePoint = true;
    editor.WndProc(SCI_SETSAVEPOINT, 0U, 0U);
    frame->drawView();
}

/////////////////////////////////////////////////////////////////////////
// File opening and saving.

#include <memory>
#include <fstream>

// Note: the 'fatalError' variable set here is later checked in valid() for
// command cmValid. If there was an error, valid() will return False,
// thus resulting in the EditorWindow being destroyed in checkValid().

void EditorWindow::tryLoadFile(bool canFail)
{
    if (!file.empty()) {
        std::error_code ec;
        file.assign(std::filesystem::absolute(file, ec));
        if (ec) {
            fatalError = true;
            showError(fmt::format("'{}' is not a valid path.", file.native()));
        } else
            fatalError = !loadFile(canFail);
        if (!fatalError)
            type.detect(*this);
    }
}

bool EditorWindow::loadFile(bool canFail)
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
            std::unique_ptr<char[]> buffer {new char[readSize]};
            sptr_t wParam = reinterpret_cast<sptr_t>(buffer.get());
            while (fSize > 0 && (ok = bool(f.read(buffer.get(), readSize)))) {
                props.analyze({buffer.get(), readSize});
                editor.WndProc(SCI_APPENDTEXT, readSize, wParam);
                fSize -= readSize;
                if (fSize < readSize)
                    readSize = fSize;
            };
            if (!ok) {
                showError(fmt::format("An error occurred while reading from file '{}'.", file.native()));
                return false;
            }
        }
    } else if (!canFail) {
        showError(fmt::format("Unable to open file '{}'.", file.native()));
        return false;
    }
    return true;
}

bool EditorWindow::trySaveFile()
{
    if (!inSavePoint || file.empty()) {
        if (file.empty()) {
            return saveAsDialog(); // Already takes care of updating the title.
        } else if (saveFile()) {
            setSavePointReached(); // Update title if necessary.
            setSavePoint(); // Notify Scintilla.
            return true;
        }
        return false;
    }
    return true;
}

bool EditorWindow::saveFile()
{
    std::ofstream f(file, ios::out | ios::binary);
    if (f) {
        size_t bytesLeft = editor.WndProc(SCI_GETTEXT, 0, 0) - 1;
        if (bytesLeft) {
            bool ok = true;
            constexpr size_t blockSize = 1 << 20; // Write in chunks of 1 MiB.
            size_t writeSize = std::min(bytesLeft, blockSize);
            std::unique_ptr<char[]> buffer {new char[writeSize]};
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
                showError(fmt::format("An error occurred while writing to file '{}'.", file.native()));
                return false;
            }
        }
    } else {
        showError(fmt::format("Unable to write to file '{}'.", file.native()));
        return false;
    }
    return true;
}

bool EditorWindow::saveAsDialog()
{
    if (TurboApp::app) {
        bool saved = false;
        TurboApp::app->openFileDialog("*.*", "Save file as", "~N~ame", fdOKButton, 0,
            [this, &saved] (TView *dialog) {
                std::filesystem::path prevFile = std::move(file);
                char fileName[MAXPATH];
                dialog->getData(fileName);
                std::error_code ec;
                file = std::filesystem::absolute(fileName, ec);
                if (ec)
                    showError(fmt::format("'{}' is not a valid path.", fileName));
                else if (canOverwrite() && saveFile()) {
                    // Saving has succeeded, now update the title.
                    TurboApp::app->updateEditorTitle(this, prevFile.native());
                    setSavePoint();
                    type.detect(*this);
                    return saved = true;
                }
                // Restore the old file path.
                file = std::move(prevFile);
                return false;
            }
        );
        return saved;
    }
    return false;
}

bool EditorWindow::canOverwrite() const
{
    std::error_code ec;
    if (std::filesystem::exists(file, ec)) {
        auto &&text = fmt::format("'{}' already exists. Overwrite?", file.native());
        return cmYes == messageBox(text.c_str(), mfConfirmation | mfYesButton | mfNoButton);
    }
    return true;
}

bool EditorWindow::tryClose()
{
    if (!inSavePoint) {
        auto &&msg = file.empty() ? fmt::format("Save '{}'?", name)
                                  : fmt::format("'{}' has been modified. Save?", file.native());
        switch (messageBox(msg.c_str(), mfConfirmation | mfYesNoCancel)) {
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

void EditorWindow::showError(const std::string &s)
{
    messageBox(s.c_str(), mfError | mfOKButton);
}

#define cpEditorWindow \
    "\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93\x94\x95\x96\x97" \
    "\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7"

TPalette& EditorWindow::getPalette() const
{
    static TPalette palette(cpEditorWindow, sizeof(cpEditorWindow) - 1);
    return palette;
}
