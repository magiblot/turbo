#ifndef TURBO_SEARCHBOX_H
#define TURBO_SEARCHBOX_H

#define Uses_TGroup
#define Uses_TPalette
#define Uses_TInputLine
#define Uses_TValidator
#include <tvision/tv.h>

#include <turbo/editstates.h>
#include "cmds.h"

namespace turbo
{
class Editor;
}

class CheckBox;
class ComboBox;

struct SearchState
{
    char findText[256] {0};
    turbo::SearchSettings settings;
};

class Searcher
{
    turbo::Editor &editor;
    turbo::SearchSettings &settings;

public:

    Searcher(turbo::Editor &aEditor, turbo::SearchSettings &aSettings) noexcept :
        editor(aEditor),
        settings(aSettings)
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

#endif
