#ifndef TVEDIT_FILEEDIT_H
#define TVEDIT_FILEEDIT_H

#include "docview.h"
#include <string>
#include <filesystem>
#include <fmt/core.h>

struct FileEditor : public DocumentView {

    template<class ...Args>
    FileEditor(Args&& ...args);

    void tryLoadFile();
    void loadFile();

};

template<class ...Args>
inline FileEditor::FileEditor(Args&& ...args) :
    DocumentView(std::forward<Args>(args)...)
{
    tryLoadFile();
}

#endif
