#ifndef TURBO_FILEEDITOR_H
#define TURBO_FILEEDITOR_H

#include <turbo/editor.h>
#include <string>
#include <utility>

namespace turbo {

struct FileEditor;

struct FileDialogs
{
    virtual ushort confirmSaveUntitled(FileEditor &) noexcept = 0;
    virtual ushort confirmSaveModified(FileEditor &) noexcept = 0;
    virtual ushort confirmOverwrite(const char *path) noexcept = 0;
    virtual void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept = 0;
    virtual bool renameError(const char *dst, const char *src, const char *cause) noexcept = 0;
    virtual bool fileTooBigError(const char *path, size_t size) noexcept = 0;
    virtual bool readError(const char *path, const char *cause) noexcept = 0;
    virtual bool writeError(const char *path, const char *cause) noexcept = 0;
    virtual bool openForReadError(const char *path, const char *cause) noexcept = 0;
    virtual bool openForWriteError(const char *path, const char *cause) noexcept = 0;
    virtual void getOpenPath(TFuncView<bool (const char *)> accept) noexcept = 0;
    virtual void getSaveAsPath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept = 0;
    virtual void getRenamePath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept = 0;
};

struct DefaultFileDialogs : FileDialogs
{
    ushort confirmSaveUntitled(FileEditor &) noexcept override;
    ushort confirmSaveModified(FileEditor &) noexcept override;
    ushort confirmOverwrite(const char *path) noexcept override;
    void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept override;
    bool renameError(const char *dst, const char *src, const char *cause) noexcept override;
    bool fileTooBigError(const char *path, size_t size) noexcept override;
    bool readError(const char *path, const char *cause) noexcept override;
    bool writeError(const char *path, const char *cause) noexcept override;
    bool openForReadError(const char *path, const char *cause) noexcept override;
    bool openForWriteError(const char *path, const char *cause) noexcept override;
    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;
};

extern DefaultFileDialogs defFileDialogs;

struct SilentFileDialogs : FileDialogs
{
    ushort confirmSaveUntitled(FileEditor &) noexcept override;
    ushort confirmSaveModified(FileEditor &) noexcept override;
    ushort confirmOverwrite(const char *path) noexcept override;
    void removeRenamedWarning(const char *dst, const char *src, const char *cause) noexcept override;
    bool renameError(const char *dst, const char *src, const char *cause) noexcept override;
    bool fileTooBigError(const char *path, size_t size) noexcept override;
    bool readError(const char *path, const char *cause) noexcept override;
    bool writeError(const char *path, const char *cause) noexcept override;
    bool openForReadError(const char *path, const char *cause) noexcept override;
    bool openForWriteError(const char *path, const char *cause) noexcept override;
    void getOpenPath(TFuncView<bool (const char *)> accept) noexcept override;
    void getSaveAsPath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;
    void getRenamePath(FileEditor &, TFuncView<bool (const char *)> accept) noexcept override;
};

extern SilentFileDialogs silFileDialogs;

bool readFile(TScintilla &scintilla, const char *path, FileDialogs & = defFileDialogs) noexcept;
void openFile( TFuncView<TScintilla&()> createScintilla,
               TFuncView<void(TScintilla &, const char *)> accept,
               FileDialogs & = defFileDialogs ) noexcept;
bool writeFile(const char *path, TScintilla &scintilla, FileDialogs & = defFileDialogs) noexcept;
bool renameFile(const char *dst, const char *src, TScintilla &scintilla, FileDialogs & = defFileDialogs) noexcept;

struct FileEditor : Editor
{
    enum : ushort { // Notification Codes
        ncSaved = 100,
    };

    std::string filePath;

    FileEditor(TScintilla &aScintilla, std::string aFilePath) noexcept;

    void detectLanguage() noexcept;
    bool save(FileDialogs & = defFileDialogs) noexcept;
    bool saveAs(FileDialogs & = defFileDialogs) noexcept;
    bool rename(FileDialogs & = defFileDialogs) noexcept;
    bool close(FileDialogs & = defFileDialogs) noexcept;

    void beforeSave() noexcept;
    virtual void afterSave() noexcept;
    void notifyAfterSave() noexcept;

};

inline FileEditor::FileEditor(TScintilla &aScintilla, std::string aFilePath) noexcept :
    Editor(aScintilla),
    filePath(std::move(aFilePath))
{
    detectLanguage();
}

inline void FileEditor::detectLanguage() noexcept
{
    theming.detectLanguage(filePath.c_str(), scintilla);
}

} // namespace turbo

#endif
