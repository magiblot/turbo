#ifndef TURBO_EDITWINDOW_H
#define TURBO_EDITWINDOW_H

#define Uses_TWindow
#define Uses_TPalette
#include <tvision/tv.h>

#include <turbo/turbo.h>
#include "util.h"

struct FileNumberState
{
    active_counter *counter;
    uint number;

    FileNumberState(active_counter &aCounter);
    ~FileNumberState();
    FileNumberState &operator=(FileNumberState &&other);
};

struct TitleState
{
    active_counter *counter;
    uint number;
    bool inSavePoint;

    bool operator!=(const TitleState &other) const
    {
        return !( counter == other.counter && number == other.number &&
                  inSavePoint == other.inSavePoint );
    }
};

struct EditorWindow;

struct EditorWindowParent
{
    virtual void handleFocus(EditorWindow &w) = 0;
    virtual void handleTitleChange(EditorWindow &w) = 0;
    virtual void removeEditor(EditorWindow &w) = 0;

    std::string fileDialogDir;
};

struct EditorWindow : public TWindow, turbo::EditorParent
{
    enum { leftMarginSep = 1 };
    static constexpr TPoint minSize {24, 6};

    turbo::FileEditorState editorState;
    list_head<EditorWindow> listHead;
    FileNumberState fileNumber;
    EditorWindowParent &parent;
    TitleState lastTitleState {};
    std::string title;
    TCommandSet enabledCmds, disabledCmds;

    static TFrame* initFrame(TRect bounds);

    EditorWindow( const TRect &bounds, turbo::Editor &editor,
                  const char *filePath, active_counter &fileCounter,
                  EditorWindowParent &aParent );

    void shutDown() override;
    void handleEvent(TEvent &ev) override;
    void setState(ushort aState, Boolean enable) override;
    Boolean valid(ushort command) override;
    void dragView(TEvent& event, uchar mode, TRect& limits, TPoint minSize, TPoint maxSize) override;
    const char *getTitle(short = 0) override;
    void sizeLimits(TPoint &min, TPoint &max) override;
    void updateCommands();
    void handleNotification(ushort, turbo::EditorState &) override;
    TPalette& getPalette() const override;

    auto &filePath() { return editorState.filePath; }

};

inline FileNumberState::FileNumberState(active_counter &aCounter) :
    counter(&aCounter),
    number(++aCounter)
{
}

inline FileNumberState::~FileNumberState()
{
    --*counter;
}

inline FileNumberState &FileNumberState::operator=(FileNumberState &&other)
{
    std::swap(counter, other.counter);
    std::swap(number, other.number);
    return *this;
}

#endif
