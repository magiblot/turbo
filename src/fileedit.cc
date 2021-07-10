#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/turbo.h>

#include <fmt/core.h>
#include <memory>
#include <fstream>
#include <tvision/compat/io.h>
#include <stdlib.h>
#include <errno.h>
#include "util.h"

namespace turbo {

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
        if (bytesLeft > 0)
        {
            bool ok = true;
            size_t readSize;
            while ( readSize = min(bytesLeft, sizeof(ioBuffer)),
                    bytesLeft > 0 && (ok = (bool) f.read(ioBuffer, readSize)) )
            {
                props.analyze({ioBuffer, readSize});
                editor.WndProc(SCI_APPENDTEXT, readSize, (sptr_t) ioBuffer);
                bytesLeft -= readSize;
            }
            if (!ok)
                return fmt::format("Cannot read from file '{}': {}.", path, strerror(errno));
        }
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

struct CwdGuard
{
    char *lastCwd;
    CwdGuard(const char *newCwd)
    {
        if (newCwd)
        {
            lastCwd = getcwd(nullptr, 0);
            int r = chdir(newCwd); (void) r;
        }
        else
            lastCwd = nullptr;
    }
    ~CwdGuard()
    {
        if (lastCwd)
        {
            int r = chdir(lastCwd); (void) r;
            ::free(lastCwd);
        }
    }
};

OpenFileWithDialogResult openFileWithDialog(const char *dir)
{
    using namespace constants;
    // TFileDialog relies on the current working directory.
    CwdGuard cwd(dir);
    // MAXPATH as assumed by TFileDialog.
    char path[MAXPATH];
    Editor *editor = nullptr;
    openFileDialog("*.*", "Open file", "~N~ame", fdOpenButton, 0,
        [&] (TView *dialog) {
            dialog->getData(path);
            return (editor = openFile(path, ofShowError));
        }
    );
    return {editor, path};
}

} // namespace turbo
