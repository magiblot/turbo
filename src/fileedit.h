#ifndef TVEDIT_FILEEDIT_H
#define TVEDIT_FILEEDIT_H

#include "docview.h"
#include <string>

struct FileEditor : public DocumentView {

    char fileName[MAXPATH];

    template<class ...Args>
    FileEditor(std::string_view aFileName, Args&& ...args);

    void setFileName(std::string_view aFileName);
    void loadFile();

};

template<class ...Args>
inline FileEditor::FileEditor(std::string_view aFileName, Args&& ...args) :
    DocumentView(std::forward<Args>(args)...)
{
    strnzcpy(fileName, aFileName, MAXPATH);
    fexpand(fileName);
    loadFile();
}

#endif
