#define Uses_MsgBox
#include <tvision/tv.h>

#include <turbo/turbo.h>

#include <fmt/core.h>
#include <memory>
#include <fstream>
#include <errno.h>

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
        editor.WndProc(SCI_ALLOCATE, bytesLeft + 1000, 0U);
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
        editor.WndProc(SCI_EMPTYUNDOBUFFER, 0U, 0U);
    }
    else
        return fmt::format("Unable to open file '{}': {}.", path, strerror(errno));
    return {};
}

Editor *loadFile(const char *path, ushort options)
{
    using namespace constants;
    auto editor = std::make_unique<Editor>();
    auto &&errorMsg = loadFile(*editor, path);
    if (!errorMsg.empty())
    {
        if (options & lfShowError)
            messageBox(errorMsg, mfError | mfOKButton);
        return nullptr;
    }
    return editor.release();
}

} // namespace turbo
