#include "fileedit.h"
#include <memory>
#include <fstream>

void FileEditor::loadFile()
{
    std::ifstream f(fileName, ios::in | ios::binary);
    if (f) {
        // TODO: handle errors.
        f.seekg(0, ios::end);
        size_t fSize = f.tellg();
        f.seekg(0);
        // Allocate 1000 extra bytes, as in SciTE.
        editor.WndProc(SCI_ALLOCATE, fSize + 1000, 0U);
        if (fSize > (1 << 20))
            // Disable word wrap on big files.
            editor.WndProc(SCI_SETWRAPMODE, SC_WRAP_NONE, 0U);
        if (fSize) {
            const size_t blockSize = 1 << 20; // Read in chunks of 1 MiB.
            std::unique_ptr<char[]> buffer {new char[std::min(fSize, blockSize)]};
            sptr_t wParam = reinterpret_cast<sptr_t>(buffer.get());
            do {
                const size_t readSize = std::min(fSize, blockSize);
                f.read(buffer.get(), readSize);
                editor.WndProc(SCI_APPENDTEXT, readSize, wParam);
                fSize -= readSize;
            } while (fSize > 0);
        }
    }
}
