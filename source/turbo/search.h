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
    char replaceText[256] {0};
};

class SearchBox : public TGroup
{
    SearchState &searchState;
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

class SearchInputLine : public TInputLine
{
    char *backupData;

    void handleEvent(TEvent &ev) override;
    void shutDown() override;

public:

    SearchInputLine(const TRect &bounds, char (&aData)[256]) noexcept;
};

class SearchValidator : public TValidator
{
    SearchInputLine &inputLine;

    Boolean isValidInput(char *, Boolean) override;

public:

    SearchValidator(SearchInputLine &aInputLine) noexcept :
        inputLine(aInputLine)
    {
    }
};

#endif // TURBO_SEARCH_H
