#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/turbo.h>
#include <turbo/tpath.h>

#include <fmt/core.h>
#include <memory>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <turbo/util.h>

namespace turbo {

class PropertyDetector
{
    enum : uint {
        ndEOL = 0x0001,
    };

    uint notDetected {ndEOL};
    int eolType {SC_EOL_LF}; // Default EOL type is LF.

public:

    void analyze(TStringView text);
    void apply(TScintilla &scintilla) const;

};

void PropertyDetector::analyze(TStringView text)
{
    if (text.size())
    {
        char cur = text[0];
        char next = text.size() > 0 ? text[1] : '\0';
        size_t i = 1;
        while (notDetected)
        {
            if (notDetected & ndEOL)
            {
                if (cur == '\r' && next == '\n')
                    eolType = SC_EOL_CRLF, notDetected &= ~ndEOL;
                else if (cur == '\n')
                    eolType = SC_EOL_LF, notDetected &= ~ndEOL;
                else if (cur == '\r')
                    eolType = SC_EOL_CR, notDetected &= ~ndEOL;
            }
            if (++i < text.size())
            {
                cur = next;
                next = text[i];
            }
            else
                break;
        }
    }
}

void PropertyDetector::apply(TScintilla &scintilla) const
{
    call(scintilla, SCI_SETEOLMODE, eolType, 0U);
}

static thread_local char ioBuffer alignas(4*1024) [128*1024];

bool readFile(TScintilla &scintilla, const char *path, FileDialogs &dlgs) noexcept
// Pre: 'scintilla' has no text in it.
{
    using std::ios;
    std::ifstream f(path, ios::in | ios::binary);
    if (f)
    {
        f.seekg(0, ios::end);
        size_t bytesLeft = f.tellg();
        f.seekg(0);
        // Allocate 1000 extra bytes, like SciTE does.
        try
        {
            call(scintilla, SCI_ALLOCATE, bytesLeft + 1000, 0U);
        }
        catch (const std::bad_alloc &)
        {
            return dlgs.fileTooBigError(path, bytesLeft);
        }
        PropertyDetector props;
        bool ok = true;
        size_t readSize;
        while ( readSize = min(bytesLeft, sizeof(ioBuffer)),
                readSize > 0 && (ok = (bool) f.read(ioBuffer, readSize)) )
        {
            props.analyze({ioBuffer, readSize});
            call(scintilla, SCI_APPENDTEXT, readSize, (sptr_t) ioBuffer);
            bytesLeft -= readSize;
        }
        if (!ok)
            return dlgs.readError(path, strerror(errno));
        props.apply(scintilla);
    }
    else
        return dlgs.openForReadError(path, strerror(errno));
    return true;
}

void openFile( TFuncView<TScintilla&()> createScintilla,
               TFuncView<void(TScintilla &, const char *)> accept, FileDialogs &dlgs ) noexcept
{
    dlgs.getOpenPath([&] (const char *path) {
        auto &scintilla = createScintilla();
        if (readFile(scintilla, path, dlgs))
        {
            accept(scintilla, path);
            return true;
        }
        destroyScintilla(scintilla);
        return false;
    });
}

bool writeFile(const char *path, TScintilla &scintilla, FileDialogs &dlgs) noexcept
{
    using std::ios;
    std::ofstream f(path, ios::out | ios::binary);
    if (f)
    {
        size_t length = call(scintilla, SCI_GETLENGTH, 0U, 0U);
        bool ok = true;
        size_t writeSize;
        size_t written = 0;
        do {
            writeSize = min(length - written, sizeof(ioBuffer));
            call(scintilla, SCI_SETTARGETRANGE, written, written + writeSize);
            call(scintilla, SCI_GETTARGETTEXT, 0U, (sptr_t) ioBuffer);
            written += writeSize;
        } while (writeSize > 0 && (ok = (bool) f.write(ioBuffer, writeSize)));
        if (!ok)
            return dlgs.writeError(path, strerror(errno));
    }
    else
        return dlgs.openForWriteError(path, strerror(errno));
    return true;
}

bool renameFile(const char *dst, const char *src, TScintilla &scintilla, FileDialogs &dlgs) noexcept
{
    // Try saving first, then renaming.
    if (writeFile(src, scintilla, silFileDialogs) && ::rename(src, dst) == 0)
        return true;
    // If the above doesn't work, try saving at the new location, and then remove
    // the old file.
    else if (writeFile(dst, scintilla, silFileDialogs))
    {
        if (TPath::exists(src) && ::remove(src) != 0)
            dlgs.removeRenamedWarning(dst, src, strerror(errno));
        return true;
    }
    return dlgs.renameError(dst, src, strerror(errno));
}

bool FileEditor::save(FileDialogs &dlgs) noexcept
{
    if (filePath.empty())
        return saveAs(dlgs);
    beforeSave();
    if (writeFile(filePath.c_str(), scintilla, dlgs))
    {
        notifyAfterSave();
        return true;
    }
    return false;
}

bool FileEditor::saveAs(FileDialogs &dlgs) noexcept
{
    bool ok = false;
    dlgs.getSaveAsPath(*this, [&] (const char *path) {
        beforeSave();
        if (writeFile(path, scintilla, dlgs))
        {
            filePath = path;
            notifyAfterSave();
            return (ok = true);
        }
        return false;
    });
    return ok;
}

bool FileEditor::rename(FileDialogs &dlgs) noexcept
{
    if (filePath.empty())
        return saveAs(dlgs);
    bool ok = false;
    dlgs.getRenamePath(*this, [&] (const char *path) {
        beforeSave();
        if (renameFile(path, filePath.c_str(), scintilla, dlgs))
        {
            filePath = path;
            notifyAfterSave();
            return (ok = true);
        }
        return false;
    });
    return ok;
}

bool FileEditor::close(FileDialogs &dlgs) noexcept
{
    if (!inSavePoint())
    {
        auto reply = filePath.empty() ?
            dlgs.confirmSaveUntitled(*this) :
            dlgs.confirmSaveModified(*this);
        return (reply == cmYes && save(dlgs)) || reply == cmNo;
    }
    return true;
}

void FileEditor::beforeSave() noexcept
{
    if (!inSavePoint() && !call(scintilla, SCI_CANREDO, 0U, 0U))
    {
        call(scintilla, SCI_BEGINUNDOACTION, 0U, 0U);
        stripTrailingSpaces(scintilla);
        ensureNewlineAtEnd(scintilla);
        call(scintilla, SCI_ENDUNDOACTION, 0U, 0U);
    }
}

void FileEditor::afterSave() noexcept
{
    call(scintilla, SCI_SETSAVEPOINT, 0U, 0U);
    detectLanguage();
}

void FileEditor::notifyAfterSave() noexcept
{
    afterSave();
    if (parent)
        parent->handleNotification(ncSaved, *this);
}

DefaultFileDialogs defFileDialogs;

ushort DefaultFileDialogs::confirmSaveUntitled(FileEditor &) noexcept
{
    return messageBox("Save untitled file?", mfConfirmation | mfYesNoCancel);
}

ushort DefaultFileDialogs::confirmSaveModified(FileEditor &editor) noexcept
{
    return messageBox( fmt::format("'{}' has been modified. Save?", editor.filePath),
                       mfConfirmation | mfYesNoCancel );
}

ushort DefaultFileDialogs::confirmOverwrite(const char *path) noexcept
{
    return messageBox( fmt::format("'{}' already exists. Overwrite?", path),
                       mfConfirmation | mfYesButton | mfNoButton );
}

void DefaultFileDialogs::removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept
{
    messageBox( fmt::format("'{}' was created successfully, but '{}' could not be removed: {}.", dst, src, cause),
                mfWarning | mfOKButton );
}

bool DefaultFileDialogs::renameError(const char *dst, const char *src, const char *cause) noexcept
{
    return messageBox( fmt::format("Unable to rename '{}' into '{}': {}.", src, dst, cause),
                       mfError | mfOKButton ), false;
}

bool DefaultFileDialogs::fileTooBigError(const char *path, size_t size) noexcept
{
    return messageBox( fmt::format("Unable to open file '{}': file too big ({} bytes).", path, size),
                       mfError | mfOKButton ), false;
}

bool DefaultFileDialogs::readError(const char *path, const char *cause) noexcept
{
    return messageBox( fmt::format("Cannot read from file '{}': {}.", path, cause),
                       mfError | mfOKButton ), false;
}

bool DefaultFileDialogs::writeError(const char *path, const char *cause) noexcept
{
    return messageBox( fmt::format("Cannot write into file '{}': {}.", path, cause),
                       mfError | mfOKButton ), false;
}

bool DefaultFileDialogs::openForReadError(const char *path, const char *cause) noexcept
{
    return messageBox( fmt::format("Unable to open file '{}' for read: {}.", path, cause),
                       mfError | mfOKButton ), false;
}

bool DefaultFileDialogs::openForWriteError(const char *path, const char *cause) noexcept
{
    return messageBox( fmt::format("Unable to open file '{}' for write: {}.", path, cause),
                       mfError | mfOKButton ), false;
}

void DefaultFileDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    openFileDialog("*.*", "Open file", "~N~ame", fdOpenButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        return accept(path);
    });
}

static bool canOverwrite(FileDialogs &dlgs, const char *path) noexcept
{
    return !TPath::exists(path) || dlgs.confirmOverwrite(path) == cmYes;
}

void DefaultFileDialogs::getSaveAsPath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    auto &&title = editor.filePath.empty() ? "Save untitled file" : fmt::format("Save file '{}' as", TPath::basename(editor.filePath));
    openFileDialog("*.*", title, "~N~ame", fdOKButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        return canOverwrite(*this, path) && accept(path);
    });
}

void DefaultFileDialogs::getRenamePath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept
{
    char path[MAXPATH];
    auto &&title = fmt::format("Rename file '{}'", TPath::basename(editor.filePath));
    openFileDialog("*.*", title, "~N~ame", fdOKButton, 0, [&] (TView *dialog) {
        dialog->getData(path);
        fexpand(path);
        // Don't do anything if renaming to the same file. If the user needed to
        // save the file, they would use the 'save' feature.
        return strcmp(path, editor.filePath.c_str()) == 0 ||
            (canOverwrite(*this, path) && accept(path));
    });
}

SilentFileDialogs silFileDialogs;

ushort SilentFileDialogs::confirmSaveUntitled(FileEditor &) noexcept { return cmCancel; }
ushort SilentFileDialogs::confirmSaveModified(FileEditor &editor) noexcept { return cmCancel; }
ushort SilentFileDialogs::confirmOverwrite(const char *path) noexcept { return cmCancel; }
void SilentFileDialogs::removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept {}
bool SilentFileDialogs::renameError(const char *dst, const char *src, const char *cause) noexcept { return false; }
bool SilentFileDialogs::fileTooBigError(const char *path, size_t size) noexcept { return false; }
bool SilentFileDialogs::readError(const char *path, const char *cause) noexcept { return false; }
bool SilentFileDialogs::writeError(const char *path, const char *cause) noexcept { return false; }
bool SilentFileDialogs::openForReadError(const char *path, const char *cause) noexcept { return false; }
bool SilentFileDialogs::openForWriteError(const char *path, const char *cause) noexcept { return false; }
void SilentFileDialogs::getOpenPath(TFuncView<bool (const char *)> accept) noexcept {}
void SilentFileDialogs::getSaveAsPath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept {}
void SilentFileDialogs::getRenamePath(FileEditor &editor, TFuncView<bool (const char *)> accept) noexcept {}

} // namespace turbo
