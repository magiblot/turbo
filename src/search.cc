#define Uses_TLabel
#define Uses_TIndicator
#define Uses_TButton
#define Uses_TFrame
#include <tvision/tv.h>

#include "app.h"
#include "search.h"
#include "editwindow.h"

void SearchBox::init(EditorWindow &win)
{
    TRect r = win.getExtent().grow(-1, -1);
    r.a.y = r.b.y - 2;
    auto *box = new SearchBox(r, win);
    box->growMode = gfGrowAll & ~gfGrowLoX;
    box->options |= ofFramed | ofFirstClick | ofPreProcess | ofPostProcess;
    box->hide();
    win.insert(box);
}

SearchBox::SearchBox(const TRect &bounds, EditorWindow &win) :
    TGroup(bounds),
    visible(false)
{
    auto *findText = "~F~ind:",
         *nextText = "~N~ext",
         *prevText = "~P~revious";
    auto rLabelF = TRect(0, 0, 1 + cstrlen(findText), 1);
    auto rPrev = TRect(size.x - cstrlen(prevText) - 5, 0, size.x - 1, 2);
    auto rNext = TRect(rPrev.a.x - cstrlen(nextText) - 5, 0, rPrev.a.x, 2);
    auto rBoxF = TRect(rLabelF.b.x + 1, 0, rNext.a.x, 1);
    {
        findBox = new SearchInputLine(new Searcher(win), rBoxF, 256);
        findBox->growMode = gfGrowHiX;
        // Enable ofPostProcess so that it can receive commands even when not selected.
        findBox->options |= ofPostProcess;
        insert(findBox);
    }
    {
        TView *v = new TLabel(rLabelF, findText, findBox);
        v->growMode = 0;
        insert(v);
    }
    {
        TView *v = new TButton(rNext, nextText, cmSearchAgain, bfNormal);
        v->growMode = gfGrowLoX | gfGrowHiX;
        insert(v);
    }
    {
        TView *v = new TButton(rPrev, prevText, cmSearchPrev, bfNormal);
        v->growMode = gfGrowLoX | gfGrowHiX;
        insert(v);
    }
}

void SearchBox::handleEvent(TEvent &ev)
{
    if (owner->phase == phPreProcess) {
        // kbEsc received while visible but not focused.
        if (visible && ev.what == evKeyDown && ev.keyDown.keyCode == kbEsc) {
            close();
            clearEvent(ev);
        }
    } else {
        bool handled = true;
        switch (ev.what) {
            case evKeyDown:
                switch (ev.keyDown.keyCode) {
                    case kbEsc:
                        close();
                        break;
                    // Like TWindow:
                    case kbTab:
                        focusNext(False);
                        break;
                    case kbShiftTab:
                        focusNext(True);
                        break;
                    case kbEnter:
                        if (ev.keyDown.controlKeyState & kbShift) {
                            // Shift+Enter, only works on the linux console.
                            ev.what = evCommand;
                            ev.message.command = cmSearchPrev;
                        } else {
                            // Like TDialog:
                            ev.what = evBroadcast;
                            ev.message.command = cmDefault;
                        }
                        ev.message.infoPtr = 0;
                        handled = false;
                        break;
                    default:
                        handled = false;
                }
                break;
            case evCommand:
                switch (ev.message.command) {
                    // cmFind and cmReplace do not perform any search.
                    // They only enable the view.
                    case cmFind:
                        open();
                        break;
                    case cmReplace:
                        open();
                        break;
                    default:
                        handled = false;
                }
                break;
            default:
                handled = false;
        }
        if (handled)
            clearEvent(ev);
        TGroup::handleEvent(ev);
    }
}

void SearchBox::draw()
{
    TView::draw();
    TGroup::redraw();
}

void SearchBox::open()
{
    if (!visible && owner) {
        EditorWindow &win = *(EditorWindow *) owner;
        win.lock();
        for (auto *v : {(TView *) &win.editorView, (TView *) win.docView, (TView *) win.leftMargin})
        {
            TRect r = v->getBounds();
            r.b.y -= size.y + 1;
            v->changeBounds(r);
        }
        lock();
        show();
        unlock();
        win.frame->drawView();
        win.redrawEditor();
        win.unlock();
        visible = true;
    } else
        focus();
}

void SearchBox::close()
{
    if (visible && owner) {
        EditorWindow &win = *(EditorWindow *) owner;
        win.lock();
        for (auto *v : {(TView *) &win.editorView, (TView *) win.docView, (TView *) win.leftMargin})
        {
            TRect r = v->getBounds();
            r.b.y += size.y + 1;
            v->changeBounds(r);
        }
        hide();
        win.frame->drawView();
        win.redrawEditor();
        win.unlock();
        visible = false;
    }
}

void SearchInputLine::setState(ushort aState, Boolean enable)
{
    TInputLine::setState(aState, enable);
    // When the input line is focused, get ready to search.
    if (aState == sfFocused && enable)
        searcher->targetFromCurrent();
}

void SearchInputLine::handleEvent(TEvent &ev)
{
    if ( (ev.what == evBroadcast && ev.message.command == cmDefault && owner->phase == phFocused) ||
         (ev.what == evCommand && ev.message.command == cmSearchAgain) )
    {
        searcher->targetNext();
        doSearch();
        clearEvent(ev);
    }
    else if (ev.what == evCommand && ev.message.command == cmSearchPrev)
    {
        searcher->targetPrev();
        doSearch();
        clearEvent(ev);
    }
    else if (owner->phase == phFocused)
        TInputLine::handleEvent(ev);
}

void SearchInputLine::doSearch()
{
    searcher->onDemand = true;
    // Anything but cmValid and cmCancel will trigger TValidator::validate.
    valid(cmOK);
    searcher->onDemand = false;
}

Boolean Searcher::isValid(const char *s)
{
    // Invoked from TValidator::validate, which in turn is invoked from
    // TInputLine::valid.
    if (onDemand)
        searchText(s, true);
    return True;
}

Boolean Searcher::isValidInput(char *s, Boolean)
{
    // Invoked from TInputLine::checkValid, while typing.
    direction = forward;
    typing = true;
    searchText(s, true);
    typing = false;
    return True;
}

void Searcher::targetFromCurrent()
{
    direction = forward;
    auto cur = win.editor.WndProc(SCI_GETSELECTIONSTART, 0U, 0U);
    auto end = win.editor.WndProc(SCI_GETTEXTLENGTH, 0U, 0U);
    win.editor.WndProc(SCI_SETTARGETRANGE, cur, end);
}

void Searcher::targetNext()
{
    direction = forward;
    auto end = win.editor.WndProc(SCI_GETTEXTLENGTH, 0U, 0U);
    auto start = resultEnd != -1 ? resultEnd : 0;
    win.editor.WndProc(SCI_SETTARGETRANGE, start, end);
}

void Searcher::targetPrev()
{
    direction = backwards;
    auto start = result != -1 ? result : win.editor.WndProc(SCI_GETTEXTLENGTH, 0U, 0U);
    win.editor.WndProc(SCI_SETTARGETRANGE, start, 0);
}

void Searcher::searchText(std::string_view s, bool wrap)
{
    auto start = win.editor.WndProc(SCI_GETTARGETSTART, 0U, 0U);
    auto end = win.editor.WndProc(SCI_GETTARGETEND, 0U, 0U);
    result = win.editor.WndProc(SCI_SEARCHINTARGET, s.size(), (sptr_t) s.data());
    // Restore search target as Scintilla sets it to the result text.
    win.editor.WndProc(SCI_SETTARGETRANGE, start, end);
    if (result != -1) {
        resultEnd = result + s.size();
        win.editor.WndProc(SCI_SETSEL, result, resultEnd);
        // Since we cannot prevent TInputLine from doing duplicate searches,
        // at least search from the current point.
        auto newStart = direction == forward ? result : resultEnd;
        win.editor.WndProc(SCI_SETTARGETSTART, newStart, 0U);
    } else if (wrap && !typing) { // Do not wrap while typing.
        resultEnd = -1;
        if (direction == forward) {
            if (start != 0) { // Had not searched the whole document already.
                targetNext();
                return searchText(s, false);
            }
        } else {
            auto docEnd = win.editor.WndProc(SCI_GETTEXTLENGTH, 0U, 0U);
            if (start != docEnd) {
                targetPrev();
                return searchText(s, false);
            }
        }
    } else {
        auto cur = win.editor.WndProc(SCI_GETCURRENTPOS, 0U, 0U);
        win.editor.WndProc(SCI_SETEMPTYSELECTION, cur, 0U);
        resultEnd = -1;
    }
    win.redrawEditor();
}
