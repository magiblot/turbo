#ifndef TVEDIT_FILEEDIT_H
#define TVEDIT_FILEEDIT_H

#include "docview.h"
#include <string>
#include <filesystem>
#include <fmt/core.h>

struct FileEditor : public DocumentView {

    template<class ...Args>
    FileEditor(Args&& ...args);

    void loadFile();

};

template<class ...Args>
inline FileEditor::FileEditor(Args&& ...args) :
    DocumentView(std::forward<Args>(args)...)
{
    // fileName stored in the window as it has a longer lifetime.
    auto &fileName = window.file;
    if (!fileName.empty()) {
        std::error_code ec;
        fileName.assign(std::filesystem::absolute(fileName, ec));
        if (!ec)
            loadFile();
        else
            window.error = fmt::format("'{}' is not a valid path.", fileName.native());
    }
}

#endif
