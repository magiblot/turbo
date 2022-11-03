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

class SearchBox : public TGroup
{
    turbo::Editor &editor;

    SearchBox(const TRect &bounds, turbo::Editor &aEditor) noexcept :
        TGroup(bounds),
        editor(aEditor)
    {
    }

    void handleEvent(TEvent &ev) override;

public:

    static SearchBox &create(const TRect &editorBounds, turbo::Editor &editor) noexcept;

    void open();
    bool close();
};

class Searcher : public TValidator
{
    turbo::Editor &editor;
    bool typing {false};
    sptr_t result {-1};
    sptr_t resultEnd {-1};
    enum {Forward=0, Backwards=1} direction;

    Boolean isValid(const char *) override;
    Boolean isValidInput(char *, Boolean) override;

    void searchText(TStringView, bool wrap);

public:

    Searcher(turbo::Editor &aEditor) :
        editor(aEditor)
    {
    }

    void targetFromCurrent();
    void targetNext();
    void targetPrev();
};

class SearchInputLine : public TInputLine
{
    Searcher &searcher;

    void setState(ushort aState, Boolean enable) override;
    void handleEvent(TEvent &ev) override;
    void doSearch();

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
