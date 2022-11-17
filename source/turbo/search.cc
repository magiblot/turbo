#define Uses_TKeys
#define Uses_TLabel
#define Uses_TIndicator
#define Uses_TButton
#define Uses_TFrame
#define Uses_TDrawSurface
#include <tvision/tv.h>

#include "app.h"
#include "search.h"
#include "editwindow.h"
#include <turbo/editstates.h>
#include <turbo/util.h>

SearchBox &SearchBox::create(const TRect &editorBounds, turbo::Editor &editor) noexcept
{
    TRect bounds = editorBounds;
    bounds.a.y = bounds.b.y;
    auto &self = *new SearchBox(bounds, editor);
    self.growMode = gfGrowAll & ~gfGrowLoX;
    self.options |= ofFramed | ofFirstClick;
    self.options |= ofPostProcess; // So that search commands reach the input line.
    self.hide();

    auto *bckgrnd = new TView({0, 0, self.size.x, 2});
    bckgrnd->growMode = gfGrowHiX | gfGrowHiY;
    bckgrnd->eventMask = 0;
    self.insert(bckgrnd);

    auto *findText = "~F~ind:",
         *nextText = "~N~ext",
         *prevText = "~P~revious";
    TRect rLabelF {0, 0, 1 + cstrlen(findText), 1};
    TRect rPrev {self.size.x - cstrlen(prevText) - 5, 0, self.size.x - 1, 2};
    TRect rNext {rPrev.a.x - cstrlen(nextText) - 5, 0, rPrev.a.x, 2};
    TRect rBoxF {rLabelF.b.x + 1, 0, rNext.a.x, 1};

    auto *ilFind = new SearchInputLine(*new Searcher(editor), rBoxF, 256);
    ilFind->growMode = gfGrowHiX;
    self.insert(ilFind);

    auto *lblFind = new TLabel(rLabelF, findText, ilFind);
    lblFind->growMode = 0;
    self.insert(lblFind);

    auto *btnNext = new TButton(rNext, nextText, cmSearchAgain, bfNormal);
    btnNext->growMode = gfGrowLoX | gfGrowHiX;
    self.insert(btnNext);

    auto *btnPrev = new TButton(rPrev, prevText, cmSearchPrev, bfNormal);
    btnPrev->growMode = gfGrowLoX | gfGrowHiX;
    self.insert(btnPrev);

    self.inputLine = ilFind;

    return self;
}

void SearchBox::shutDown()
{
    inputLine = nullptr;
    TGroup::shutDown();
}

void SearchBox::handleEvent(TEvent &ev)
{
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
    TGroup::handleEvent(ev);
}

static inline void fixEditorSize(SearchBox &self, turbo::Editor &editor)
{
    turbo::forEachNotNull([&] (TView &v) {
        TRect r = v.getBounds();
        r.b.y = self.origin.y - (self.size.y > 0);
        v.setBounds(r);
    }, editor.view, editor.leftMargin);
    editor.redraw();
}

void SearchBox::open()
{
    if (inputLine)
        inputLine->select();
    if (size.y == 0)
    {
        TRect r = getBounds();
        r.a.y = r.b.y - 2;
        changeBounds(r);
        show();
        fixEditorSize(*this, editor);
    }
    else
        focus();
}

bool SearchBox::close()
{
    if (size.y != 0)
    {
        hide();
        TRect r = getBounds();
        r.a.y = r.b.y;
        changeBounds(r);
        fixEditorSize(*this, editor);
        return true;
    }
    return false;
}

void SearchInputLine::handleEvent(TEvent &ev)
{
    using namespace turbo;
    if (owner->phase == phFocused || ev.what == evCommand)
    {
        if ( (ev.what == evCommand && ev.message.command == cmSearchAgain) ||
             (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter)) )
        {
            searcher.searchText(data, sdForward);
            clearEvent(ev);
        }
        else if ( (ev.what == evCommand && ev.message.command == cmSearchPrev) ||
                  (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter, kbShift)) )
        {
            searcher.searchText(data, sdBackwards);
            clearEvent(ev);
        }
        else
            TInputLine::handleEvent(ev);
    }
}

Boolean Searcher::isValidInput(char *s, Boolean)
{
    // Invoked while typing.
    using namespace turbo;
    searchText(s, sdForwardIncremental);
    return True;
}

void Searcher::searchText(TStringView s, turbo::SearchDirection direction)
{
    using namespace turbo;
    search(editor.scintilla, s, direction, SearchSettings {});
    editor.partialRedraw();
}
