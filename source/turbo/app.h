#ifndef TURBO_APP_H
#define TURBO_APP_H

#define Uses_TApplication
#define Uses_TFileDialog
#include <tvision/tv.h>

#include <turbo/clipboard.h>
#include "doctree.h"
#include "apputils.h"
#include "editwindow.h"

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
    cmCloseEditor   = 109,
    cmRename        = 110;

// Commands that cannot be deactivated.
const ushort
    cmToggleTree    = 1000;

struct TurboApp : public TApplication, EditorWindowParent
{

    FileCounter fileCount;
    list_head<EditorWindow> MRUlist;
    TClockView *clock;
    DocumentTreeWindow *docTree;
    TCommandSet editorCmds;
    bool argsParsed {false};
    int argc;
    const char **argv;
    turbo::SystemClipboard clipboard;
    std::string mostRecentDir;

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

    TPalette& getPalette() const override;
    void updatePalette(EditorWindow &w) const;

};

enum : uchar
{
    edFramePassive = 136,
    edFrameActive,
    edFrameIcon,
    edScrollBarPageArea,
    edScrollBarControls,
    edStaticText,
    edLabelNormal,
    edLabelSelected,
    edLabelShortcut,
    edButtonNormal,
    edButtonDefault,
    edButtonSelected,
    edButtonDisabled,
    edButtonShortcut,
    edButtonShadow,
    edClusterNormal,
    edClusterSelected,
    edClusterShortcut,
    edInputLineNormal,
    edInputLineSelected,
    edInputLineArrows,
    edHistoryArrow,
    edHistorySides,
    edHistWinScrollBarPageArea,
    edHistWinScrollBarControls,
    edListViewerNormal,
    edListViewerFocused,
    edListViewerSelected,
    edListViewerDivider,
    edInfoPane,
    edClusterDisabled,
    edReserved,
};

#endif
