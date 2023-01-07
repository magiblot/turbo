#define Uses_TKeys
#define Uses_TEvent
#define Uses_TLabel
#define Uses_TIndicator
#define Uses_TButton
#define Uses_TFrame
#define Uses_TDrawSurface
#define Uses_TCheckBoxes
#define Uses_TSItem
#include <tvision/tv.h>

#include "search.h"
#include "checkbox.h"
#include "combobox.h"
#include <turbo/editstates.h>
#include <turbo/util.h>

static constexpr SpanListModelEntry<turbo::SearchMode> searchModes[] =
{
    {turbo::smPlainText, "Plain text"},
    {turbo::smWholeWords, "Whole words"},
    {turbo::smRegularExpression, "Regular expression"},
};

static constexpr SpanListModel<turbo::SearchMode> searchModeListModel {searchModes};

SearchBox::SearchBox( const TRect &bounds, turbo::Editor &editor,
                      SearchState &aSearchState ) noexcept :
    TGroup(bounds),
    searchState(aSearchState)
{
    options |= ofFramed | ofFirstClick;

    auto *bckgrnd = new TView({0, 0, size.x, height});
    bckgrnd->growMode = gfGrowHiX | gfGrowHiY;
    bckgrnd->eventMask = 0;
    insert(bckgrnd);

    auto *findText = "~F~ind:";
    auto *nextText = "~N~ext";
    auto *prevText = "~P~revious";
    auto *modeText = "~M~ode:";
    auto *caseText = "~C~ase sensitive";
    TRect rLabelF {0, 0, 1 + cstrlen(findText), 1};
    TRect rPrev {size.x - 1 - cstrlen(prevText) - 4, 0, size.x - 1, 2};
    TRect rNext {rPrev.a.x - cstrlen(nextText) - 4, 0, rPrev.a.x, 2};
    TRect rBoxF {rLabelF.b.x + 1, 0, rNext.a.x, 1};
    TRect rCase {size.x - cstrlen(caseText) - 6, 2, size.x, 3};
    TRect rLabelM {0, 2, 1 + cstrlen(modeText), 3};
    TRect rBoxM {rLabelM.b.x + 1, 2, rCase.a.x, 3};

    auto *ilFind = new SearchInputLine(rBoxF, searchState.findText);
    ilFind->growMode = gfGrowHiX;
    ilFind->selectAll(true);

    auto *lblFind = new TLabel(rLabelF, findText, ilFind);
    lblFind->growMode = 0;
    insert(lblFind);

    auto *btnNext = new TButton(rNext, nextText, cmSearchAgain, bfNormal);
    btnNext->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnNext);

    auto *btnPrev = new TButton(rPrev, prevText, cmSearchPrev, bfNormal);
    btnPrev->growMode = gfGrowLoX | gfGrowHiX;
    insert(btnPrev);

    cmbMode = new ComboBox(rBoxM, searchModeListModel);
    cmbMode->growMode = gfGrowHiX;
    insert(cmbMode);

    auto *lblMode = new TLabel(rLabelM, modeText, cmbMode);
    lblMode->growMode = 0;
    insert(lblMode);

    cbCaseSensitive = new CheckBox(rCase, caseText);
    cbCaseSensitive->growMode = gfGrowLoX | gfGrowHiX;
    insert(cbCaseSensitive);

    // This must be the first selectable view.
    insert(ilFind);

    loadSettings();
}

void SearchBox::shutDown()
{
    cmbMode = nullptr;
    cbCaseSensitive = nullptr;
    TGroup::shutDown();
}

void SearchBox::handleEvent(TEvent &ev)
{
    TGroup::handleEvent(ev);
    if (ev.what == evKeyDown)
    {
        switch (ev.keyDown.keyCode)
        {
            case kbTab:
                focusNext(False);
                clearEvent(ev);
                break;
            case kbShiftTab:
                focusNext(True);
                clearEvent(ev);
                break;
        }
    }
    else if (ev.what == evCommand && ev.message.command == cmFindSearchBox)
        clearEvent(ev);
    else if (ev.what == evBroadcast && ev.message.command == cmStateChanged)
        storeSettings();
}

void SearchBox::loadSettings()
{
    using namespace turbo;
    if (cmbMode)
    {
        auto settings = searchState.settingsPreset.get();

        cmbMode->setCurrentIndex(settings.mode);
        cbCaseSensitive->setChecked(settings.flags & sfCaseSensitive);
    }
}

void SearchBox::storeSettings()
{
    using namespace turbo;
    if (cmbMode)
    {
        auto settings = searchState.settingsPreset.get();

        if (auto *entry = (const SpanListModelEntry<turbo::SearchMode> *) cmbMode->getCurrent())
            settings.mode = entry->data;
        settings.flags = -cbCaseSensitive->isChecked() & sfCaseSensitive;

        searchState.settingsPreset.set(settings);
    }
}

SearchInputLine::SearchInputLine(const TRect &bounds, char (&aData)[256]) noexcept :
    TInputLine(bounds, 256, new SearchValidator(*this)),
    backupData(data)
{
    data = aData;
}

void SearchInputLine::shutDown()
{
    data = backupData;
    TInputLine::shutDown();
}

void SearchInputLine::handleEvent(TEvent &ev)
{
    if (ev.what == evKeyDown && ev.keyDown.keyCode == kbEnter)
    {
        bool shift = (ev.keyDown.controlKeyState & kbShift);
        ev.what = evCommand;
        ev.message.command = shift ? cmSearchPrev : cmSearchAgain;
        ev.message.infoPtr = nullptr;
        putEvent(ev);
        clearEvent(ev);
    }
    else
        TInputLine::handleEvent(ev);
}

Boolean SearchValidator::isValidInput(char *text, Boolean)
{
    // Invoked while typing.
    TEvent ev;
    ev.what = evCommand;
    ev.message.command = cmSearchIncr;
    ev.message.infoPtr = nullptr;
    inputLine.putEvent(ev);
    return True;
}

