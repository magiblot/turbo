#ifndef TURBO_SEARCHBOX_H
#define TURBO_SEARCHBOX_H

#define Uses_TGroup
#define Uses_TPalette
#define Uses_TInputLine
#define Uses_TValidator
#include <tvision/tv.h>

namespace turbo
{
class Editor;
}

class CheckBox;

class SearchBox : public TGroup
{
    turbo::Editor &editor;
    turbo::SearchSettings &settings;
    TInputLine *inputLine {nullptr};
    CheckBox *cbCaseSensitive {nullptr};

    SearchBox(const TRect &bounds, turbo::Editor &aEditor, turbo::SearchSettings &aSettings) noexcept :
        TGroup(bounds),
        editor(aEditor),
        settings(aSettings)
    {
    }

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

    void readSettings();
    void writeSettings();

public:

    static SearchBox &create(const TRect &editorBounds, turbo::Editor &editor, turbo::SearchSettings &aSettings) noexcept;

    void open();
    bool close();
};

class Searcher : public TValidator
{
    turbo::Editor &editor;
    turbo::SearchSettings &settings;

    Boolean isValidInput(char *, Boolean) override;

public:

    Searcher(turbo::Editor &aEditor, turbo::SearchSettings &aSettings) :
        editor(aEditor),
        settings(aSettings)
    {
    }

    void searchText(TStringView, turbo::SearchDirection direction);
};

class SearchInputLine : public TInputLine
{
    Searcher &searcher;

    void handleEvent(TEvent &ev) override;

public:

    template<typename ...Args>
    SearchInputLine(Searcher &aSearcher, Args&& ...args) :
        TInputLine(args..., &aSearcher),
        searcher(aSearcher)
    {
        options |= ofPostProcess; // For search commands when not selected.
    }
};

#endif
