#define Uses_TKeys
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
#include <turbo/editstates.h>
#include <turbo/util.h>

SearchBox::SearchBox( const TRect &bounds, turbo::Editor &editor,
                      SearchState &aSearchState ) noexcept :
    TGroup(bounds),
    searchState(aSearchState),
    searcher(editor, searchState.settings)
{
    options |= ofFramed | ofFirstClick;

    auto *bckgrnd = new TView({0, 0, size.x, height});
    bckgrnd->growMode = gfGrowHiX | gfGrowHiY;
    bckgrnd->eventMask = 0;
    insert(bckgrnd);

    auto *findText = "~F~ind:",
         *nextText = "~N~ext",
         *prevText = "~P~revious",
         *caseText = "~C~ase sensitive";
    TRect rLabelF {0, 0, 1 + cstrlen(findText), 1};
    TRect rPrev {size.x - cstrlen(prevText) - 5, 0, size.x - 1, 2};
    TRect rNext {rPrev.a.x - cstrlen(nextText) - 5, 0, rPrev.a.x, 2};
    TRect rBoxF {rLabelF.b.x + 1, 0, rNext.a.x, 1};
    TRect rCase {size.x - cstrlen(caseText) - 6, 2, size.x, 3};

    auto *ilFind = new SearchInputLine(rBoxF, searchState.findText, searcher);
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

    cbCaseSensitive = new CheckBox(rCase, caseText);
    cbCaseSensitive->growMode = gfGrowLoX | gfGrowHiX;
    insert(cbCaseSensitive);

    // This must be the first selectable view.
    insert(ilFind);

    loadSettings();
}

void SearchBox::shutDown()
{
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
    else if (ev.what == evCommand)
    {
        if (ev.message.command == cmStateChanged)
        {
            storeSettings();
            clearEvent(ev);
        }
        else if (ev.message.command == cmFindSearchBox)
            clearEvent(ev);
    }
}

void SearchBox::loadSettings()
{
    using namespace turbo;
    auto &settings = searchState.settings;
    if (cbCaseSensitive)
        cbCaseSensitive->setChecked(settings.flags & sfCaseSensitive);
}

void SearchBox::storeSettings()
{
    using namespace turbo;
    auto &settings = searchState.settings;
    if (cbCaseSensitive)
        settings.flags = -cbCaseSensitive->isChecked() & sfCaseSensitive;
}

SearchInputLine::SearchInputLine(const TRect &bounds, char (&aData)[256], Searcher &aSearcher) noexcept :
    TInputLine(bounds, 256, new SearchValidator(aSearcher)),
    searcher(aSearcher),
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
    using namespace turbo;
    if (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter))
    {
        searcher.search(data, sdForward);
        clearEvent(ev);
    }
    else if (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter, kbShift))
    {
        searcher.search(data, sdBackwards);
        clearEvent(ev);
    }
    else
        TInputLine::handleEvent(ev);
}

Boolean SearchValidator::isValidInput(char *text, Boolean)
{
    // Invoked while typing.
    using namespace turbo;
    searcher.search(text, sdForwardIncremental);
    return True;
}

void Searcher::search(TStringView text, turbo::SearchDirection direction)
{
    editor.search(text, direction, settings);
    editor.partialRedraw();
}
