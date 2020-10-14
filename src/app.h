#ifndef TURBO_APP_H
#define TURBO_APP_H

#define Uses_TApplication
#define Uses_TFileDialog
#include <tvision/tv.h>

#include <string_view>
#include <unordered_map>
#include <vector>
#include "util.h"
#include "doctree.h"

struct EditorWindow;
class TClockView;

const ushort
    cmOpenRecent    = 100,
    cmEditorNext    = 101,
    cmEditorPrev    = 102,
    cmToggleWrap    = 103,
    cmToggleLineNums= 104,
    cmSearchPrev    = 105,
    cmToggleIndent  = 106,
    cmTreeNext      = 107,
    cmTreePrev      = 108,
    cmCloseEditor   = 109;

// Commands that cannot be deactivated.
const ushort
    cmToggleTree    = 1000;

struct TurboApp : public TApplication {

    std::unordered_map<std::string_view, active_counter> fileCount;
    std::vector<std::string> files; // Storage for the strings referenced by fileCount;
    list_head<EditorWindow> MRUlist;
    TClockView *clock;
    DocumentTreeWindow *docTree;
    TCommandSet editorCmds;
    bool argsParsed {false};
    int argc;
    const char **argv;
    Scintilla::SelectionText clipboard;

    static TurboApp *app;

    TurboApp(int argc=0, const char *argv[]=0);
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);

    void idle() override;
    void getEvent(TEvent &event) override;
    void handleEvent(TEvent& event) override;
    void shell();
    void parseArgs();

    void fileNew();
    void fileOpen();
    void fileSave();
    bool openEditor(std::string_view fileName, bool canFail=false);
    void closeAll();
    TRect newEditorBounds() const;
    void setEditorTitle(EditorWindow *w);
    void updateEditorTitle(EditorWindow *w, std::string_view prevFile);
    active_counter& getFileCounter(std::string_view file);
    void addEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);

    void showEditorList(TEvent *ev);
    void toggleTreeView();

    // The path of the most recently focused editor, so that file dialogs
    // are opened there.

    util::u8path mostRecentDir;
    void setFocusedEditor(EditorWindow *w); // Set from here.

    template<typename Func>
    void openFileDialog( const char *aWildCard, const char *aTitle,
                         const char *inputName, ushort aOptions,
                         uchar histId, Func &&callback );

    TPalette& getPalette() const override;

};

template<typename Func>
inline void TurboApp::openFileDialog( const char *aWildCard, const char *aTitle,
                                       const char *inputName, ushort aOptions,
                                       uchar histId, Func &&callback )
{
    // Unfortunately, TFileDialog relies on the current directory.
    [[maybe_unused]] int rr = chdir(mostRecentDir.c_str());
    auto *dialog = new TFileDialog( aWildCard, aTitle,
                                    inputName, aOptions,
                                    histId );
    execDialog(dialog, nullptr, callback);
}

#endif
