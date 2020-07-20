#ifndef TVEDIT_APP_H
#define TVEDIT_APP_H

#define Uses_TApplication
#include <tvision/tv.h>

#include <string_view>
#include <unordered_map>
#include "util.h"

class EditorWindow;
class TClockView;

const ushort
    cmOpenRecent    = 1001;

struct TVEditApp : public TApplication {

    std::unordered_map<std::string_view, active_counter> fileCount;
    std::vector<std::string> files; // Storage for the strings referenced by fileCount;
    list_head<EditorWindow> MRUlist;
    TClockView *clock;

    static TVEditApp *app;

    TVEditApp();
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);

    void idle() override;
    void handleEvent(TEvent& event) override;
    void shell();

    void fileNew();
    void fileOpen();
    void fileSave();
    bool openEditor(std::string_view fileName);
    void setEditorTitle(EditorWindow *w);
    void updateEditorTitle(EditorWindow *w, std::string_view prevFile);
    active_counter& getFileCounter(std::string_view file);
    void addEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);
    void tellFocusedEditor(EditorWindow *w);
    std::filesystem::path getMostRecentDir();
    template<typename Func>
    void openFileDialog( const char *aWildCard, const char *aTitle,
                         const char *inputName, ushort aOptions,
                         uchar histId, Func &&callback );

};

template<typename Func>
inline void TVEditApp::openFileDialog( const char *aWildCard, const char *aTitle,
                                       const char *inputName, ushort aOptions,
                                       uchar histId, Func &&callback )
{
    // Unfortunately, TFileDialog relies on the current directory.
    chdir(getMostRecentDir().c_str());
    auto *dialog = new TFileDialog( aWildCard, aTitle,
                                    inputName, aOptions,
                                    histId );
    execDialog(dialog, nullptr, callback);
}

#endif
