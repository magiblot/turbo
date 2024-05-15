#ifndef TURBO_APP_H
#define TURBO_APP_H

#define Uses_TApplication
#define Uses_TFileDialog
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include "doctree.h"
#include "apputils.h"
#include "editwindow.h"
#include "cmds.h"

struct EditorWindow;
class TClockView;

extern TCommandSet allCmUseLanguages;

struct TurboApp : public TApplication, EditorWindowParent
{
    FileCounter fileCount;
    list_head<EditorWindow> MRUlist;
    TClockView *clock;
    DocumentTreeWindow *docTree;    // Never null once the app has been initialized
    TCommandSet editorCmds;
    bool argsParsed {false};
    int argc;
    const char **argv;
    turbo::SearchSettings searchSettings;

    static TurboApp *app;

    TurboApp(int argc=0, const char *argv[]=0) noexcept;
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);

    void shutDown() override;
    void idle() override;
    void getEvent(TEvent &event) override;
    void handleEvent(TEvent& event) override;
    void parseArgs();

    void fileNew();
    void fileOpen();
    void fileOpenOrNew(const char *path);
    void closeAll();
    TRect newEditorBounds() const;
    turbo::TScintilla &createScintilla() noexcept;
    void addEditor(turbo::TScintilla &, const char *path);
    void showEditorList(TEvent *ev);
    void toggleTreeView();

    void handleFocus(EditorWindow &w) noexcept override;
    void handleTitleChange(EditorWindow &w) noexcept override;
    void removeEditor(EditorWindow &w) noexcept override;
    const char *getFileDialogDir() noexcept override;

    struct Config {
        // These are the defaults to be used if no config file exists yet.
        bool lineNumbers = true;
        bool autoIndent = true;
        bool wrapping = false;

        std::string mostRecentDir;
    } config;

    void loadConfig();
    void saveConfig();
};

#endif
