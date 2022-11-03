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

    ilFind->select();

    return self;
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

void SearchInputLine::setState(ushort aState, Boolean enable)
{
    TInputLine::setState(aState, enable);
    // When the input line is focused, get ready to search.
    if (aState == sfFocused && enable)
        searcher.targetFromCurrent();
}

void SearchInputLine::handleEvent(TEvent &ev)
{
    if (owner->phase == phFocused || ev.what == evCommand)
    {
        if ( (ev.what == evCommand && ev.message.command == cmSearchAgain) ||
             (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter)) )
        {
            searcher.targetNext();
            doSearch();
            clearEvent(ev);
        }
        else if ( (ev.what == evCommand && ev.message.command == cmSearchPrev) ||
                  (ev.what == evKeyDown && ev.keyDown == TKey(kbEnter, kbShift)) )
        {
            searcher.targetPrev();
            doSearch();
            clearEvent(ev);
        }
        else
            TInputLine::handleEvent(ev);
    }
}

void SearchInputLine::doSearch()
{
    searcher.validate(data);
}

Boolean Searcher::isValid(const char *s)
{
    // Invoked from TValidator::validate.
    if (*s)
        searchText(s, true);
    return True;
}

Boolean Searcher::isValidInput(char *s, Boolean)
{
    // Invoked from TInputLine::checkValid, while typing.
    targetNext();
    typing = true;
    searchText(s, true);
    typing = false;
    return True;
}

void Searcher::targetFromCurrent()
{
    direction = Forward;
    auto cur = editor.callScintilla(SCI_GETSELECTIONSTART, 0U, 0U);
    auto end = editor.callScintilla(SCI_GETTEXTLENGTH, 0U, 0U);
    editor.callScintilla(SCI_SETTARGETRANGE, cur, end);
}

void Searcher::targetNext()
{
    direction = Forward;
    auto end = editor.callScintilla(SCI_GETTEXTLENGTH, 0U, 0U);
    auto start = resultEnd != -1 ? resultEnd : 0;
    editor.callScintilla(SCI_SETTARGETRANGE, start, end);
}

void Searcher::targetPrev()
{
    direction = Backwards;
    auto start = result != -1 ? result : editor.callScintilla(SCI_GETTEXTLENGTH, 0U, 0U);
    editor.callScintilla(SCI_SETTARGETRANGE, start, 0);
}

void Searcher::searchText(TStringView s, bool wrap)
{
    auto start = editor.callScintilla(SCI_GETTARGETSTART, 0U, 0U);
    auto end = editor.callScintilla(SCI_GETTARGETEND, 0U, 0U);
    result = editor.callScintilla(SCI_SEARCHINTARGET, s.size(), (sptr_t) s.data());
    // Restore search target as Scintilla sets it to the result text.
    editor.callScintilla(SCI_SETTARGETRANGE, start, end);
    if (result != -1)
    {
        resultEnd = result + s.size();
        editor.callScintilla(SCI_SETSEL, result, resultEnd);
        // Since we cannot prevent TInputLine from doing duplicate searches,
        // at least search from the current point.
        auto newStart = direction == Forward ? result : resultEnd;
        editor.callScintilla(SCI_SETTARGETSTART, newStart, 0U);
    }
    else if (wrap && !typing) // Must not wrap while typing.
    {
        resultEnd = -1;
        auto docEnd = editor.callScintilla(SCI_GETTEXTLENGTH, 0U, 0U);
        auto searchEnd = direction == Forward ? 0 : docEnd;
        if (start != searchEnd) // Had not yet searched the whole document.
        {
            direction == Forward ? targetNext() : targetPrev();
            return searchText(s, false);
        }
    }
    else
    {
        resultEnd = -1;
        auto cur = editor.callScintilla(SCI_GETCURRENTPOS, 0U, 0U);
        editor.callScintilla(SCI_SETEMPTYSELECTION, cur, 0U);
    }
    editor.redraw();
}
