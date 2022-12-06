#ifndef TURBO_SEARCH_H
#define TURBO_SEARCH_H

#define Uses_TGroup
#define Uses_TInputLine
#define Uses_TValidator
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include "cmds.h"
#include "apputils.h"

namespace turbo
{
class Editor;
}

class CheckBox;
class ComboBox;

struct SearchState
{
    Preset<turbo::SearchSettings> settingsPreset;
    char findText[256] {0};
};

class Searcher
{
    turbo::Editor &editor;
    Preset<turbo::SearchSettings> &settingsPreset;

public:

    Searcher(turbo::Editor &aEditor, Preset<turbo::SearchSettings> &aSettingsPreset) noexcept :
        editor(aEditor),
        settingsPreset(aSettingsPreset)
    {
    }

    void search(TStringView text, turbo::SearchDirection direction);
};

class SearchBox : public TGroup
{
    SearchState &searchState;
    Searcher searcher;
    ComboBox *cmbMode;
    CheckBox *cbCaseSensitive;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

    void loadSettings();
    void storeSettings();

public:

    enum { findCommand = cmFindSearchBox };
    enum { height = 3 };

    SearchBox(const TRect &bounds, turbo::Editor &editor, SearchState &searchState) noexcept;
};

class SearchValidator : public TValidator
{
    Searcher &searcher;

    Boolean isValidInput(char *, Boolean) override;

public:

    SearchValidator(Searcher &aSearcher) :
        searcher(aSearcher)
    {
    }
};

class SearchInputLine : public TInputLine
{
    Searcher &searcher;
    char *backupData;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

public:

    SearchInputLine(const TRect &bounds, char (&aData)[256], Searcher &aSearcher) noexcept;
};

#endif // TURBO_SEARCH_H
