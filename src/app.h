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
    bool openEditor(std::string_view fileName);
    void setEditorTitle(EditorWindow *w);
    active_counter& getFileCounter(std::string_view file);
    void addEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);
    void tellFocusedEditor(EditorWindow *w);
    std::filesystem::path getMostRecentDir();

};

#endif
