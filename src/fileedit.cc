#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/turbo.h>

#include <fmt/core.h>
#include <memory>
#include <fstream>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include "util.h"

namespace turbo {

void FileEditorState::detectLanguage()
{
    Language lastLang = theming.language;
    if ( theming.detectLanguage(filePath.c_str(), editor) &&
         lastLang == langNone && theming.language != langNone )
        lineNumbers.enabled = true;
}

static thread_local char ioBuffer alignas(4*1024) [128*1024];

static std::string loadFile(Editor &editor, const char *path)
// Pre: 'editor' has no text in it.
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
            editor.WndProc(SCI_ALLOCATE, bytesLeft + 1000, 0U);
        }
        catch (const std::bad_alloc &)
        {
            return fmt::format("Unable to open file '{}': file too big ({} bytes).", path, bytesLeft);
        }
        DocumentProperties props;
        bool ok = true;
        size_t readSize;
        while ( readSize = min(bytesLeft, sizeof(ioBuffer)),
                readSize > 0 && (ok = (bool) f.read(ioBuffer, readSize)) )
        {
            props.analyze({ioBuffer, readSize});
            editor.WndProc(SCI_APPENDTEXT, readSize, (sptr_t) ioBuffer);
            bytesLeft -= readSize;
        }
        if (!ok)
            return fmt::format("Cannot read from file '{}': {}.", path, strerror(errno));
        props.apply(editor);
    }
    else
        return fmt::format("Unable to open file '{}': {}.", path, strerror(errno));
    return {};
}

Editor *openFile(const char *path, ushort options)
{
    using namespace constants;
    auto editor = std::make_unique<Editor>();
    auto &&errorMsg = loadFile(*editor, path);
    if (!errorMsg.empty())
    {
        if (options & ofShowError)
            messageBox(errorMsg, mfError | mfOKButton);
        return nullptr;
    }
    return editor.release();
}

OpenFileWithDialogResult openFileWithDialog()
{
    using namespace constants;
    // MAXPATH as assumed by TFileDialog.
    char path[MAXPATH];
    Editor *editor = nullptr;
    openFileDialog("*.*", "Open file", "~N~ame", fdOpenButton, 0,
        [&] (TView *dialog) {
            dialog->getData(path);
            return (editor = openFile(path, ofShowError));
        }
    );
    if (editor)
        return {editor, path};
    return {};
}

static std::string saveFile(const char *path, Editor &editor)
{
    using std::ios;
    std::ofstream f(path, ios::out | ios::binary);
    if (f)
    {
        size_t length = editor.WndProc(SCI_GETLENGTH, 0U, 0U);
        bool ok = true;
        size_t writeSize;
        size_t written = 0;
        do {
            writeSize = min(length - written, sizeof(ioBuffer));
            editor.WndProc(SCI_SETTARGETRANGE, written, written + writeSize);
            editor.WndProc(SCI_GETTARGETTEXT, 0U, (sptr_t) ioBuffer);
            written += writeSize;
        } while (writeSize > 0 && (ok = (bool) f.write(ioBuffer, writeSize)));
        if (!ok)
            return fmt::format("Cannot write into file '{}': {}.", path, strerror(errno));
    }
    else
        return fmt::format("Unable to write file '{}': {}.", path, strerror(errno));
    return {};
}

bool saveFile(const char *path, Editor &editor, ushort options)
{
    using namespace constants;
    auto &&errorMsg = saveFile(path, editor);
    if (!errorMsg.empty())
    {
        if (options & ofShowError)
            messageBox(errorMsg, mfError | mfOKButton);
        return false;
    }
    return true;
}

static bool canOverwrite(const char *path)
{
    if (TPath::exists(path))
        return messageBox(
            fmt::format("'{}' already exists. Overwrite?", path),
            mfConfirmation | mfYesButton | mfNoButton
        ) == cmYes;
    return true;
}

std::string saveFileWithDialog(Editor &editor)
{
    using namespace constants;
    char path[MAXPATH];
    bool ok = false;
    openFileDialog("*.*", "Save file as", "~N~ame", fdOKButton, 0,
        [&] (TView *dialog) {
            dialog->getData(path);
            fexpand(path);
            if (canOverwrite(path) && saveFile(path, editor, ofShowError))
                return (ok = true);
            return false;
        }
    );
    if (ok)
        return path;
    return {};
}

bool renameFile(const char *dst, const char *src, Editor &editor, ushort options)
{
    using namespace constants;
    // Try saving first, then renaming.
    if (saveFile(src, editor, 0) && ::rename(src, dst) == 0)
        return true;
    // If the above doesn't work, try saving at the new location, and then remove
    // the old file.
    else if (saveFile(dst, editor, 0))
    {
        if (TPath::exists(src) && ::remove(src) != 0 && (options & ofShowError))
            messageBox(
                fmt::format("'{}' was created successfully, but '{}' could not be removed: {}.", dst, src, strerror(errno)),
                mfWarning | mfOKButton
            );
        return true;
    }
    messageBox(
        fmt::format("Unable to rename '{}' into '{}': {}.", src, dst, strerror(errno)),
        mfError | mfOKButton
    );
    return false;
}

std::string renameFileWithDialog(const char *src, Editor &editor)
{
    using namespace constants;
    char path[MAXPATH];
    bool ok = false;
    openFileDialog(
        "*.*", fmt::format("Rename file '{}'", TPath::basename(src)),
        "~N~ame", fdOKButton, 0,
        [&] (TView *dialog) {
            dialog->getData(path);
            fexpand(path);
            // Don't do anything if renaming to the same file. If the user needed to
            // save the file, they would use the 'save' feature.
            if (strcmp(path, src) == 0)
                return true;
            if (canOverwrite(path) && renameFile(path, src, editor, ofShowError))
                return (ok = true);
            return false;
        }
    );
    if (ok)
        return path;
    return {};
}

bool FileEditorState::save()
{
    using namespace constants;
    bool success = false;
    beforeSave();
    if (filePath.empty())
        success = !(filePath = saveFileWithDialog(editor)).empty();
    else
        success = saveFile(filePath.c_str(), editor, ofShowError);
    if (success)
        afterSave();
    return success;
}

bool FileEditorState::saveAs()
{
    beforeSave();
    auto &&newFilePath = saveFileWithDialog(editor);
    if (!newFilePath.empty())
    {
        filePath = std::move(newFilePath);
        afterSave();
        return true;
    }
    return false;
}

bool FileEditorState::rename()
{
    if (filePath.empty())
        return saveAs();
    beforeSave();
    auto &&newFilePath = renameFileWithDialog(filePath.c_str(), editor);
    if (!newFilePath.empty())
    {
        filePath = std::move(newFilePath);
        afterSave();
        return true;
    }
    return false;
}

void FileEditorState::beforeSave()
{
    if (!inSavePoint() && !editor.WndProc(SCI_CANREDO, 0U, 0U))
    {
        editor.WndProc(SCI_BEGINUNDOACTION, 0U, 0U);
        stripTrailingSpaces(editor);
        ensureNewlineAtEnd(editor);
        editor.WndProc(SCI_ENDUNDOACTION, 0U, 0U);
    }
}

void FileEditorState::afterSave()
{
    editor.WndProc(SCI_SETSAVEPOINT, 0U, 0U);
    detectLanguage();
}

} // namespace turbo
