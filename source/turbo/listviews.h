#ifndef TURBO_LISTVIEWS_H
#define TURBO_LISTVIEWS_H

#define Uses_TWindow
#define Uses_TPalette
#define Uses_TListViewer
#include <tvision/tv.h>

class ListModel
{
public:
    virtual size_t size() const noexcept = 0;
    virtual void *at(size_t i) const noexcept = 0;
    virtual TStringView getText(void *) const noexcept = 0;
};

size_t maxWidth(const ListModel &model) noexcept;

class ListView;
using ListViewCreator = TFuncView<ListView *(TRect, TWindow *, const ListModel &)>;

template <class Viewer>
ListView *defListViewCreator(TRect, TWindow *, const ListModel &);

class ListWindow : public TWindow
{
    ListView *viewer;

public:

    // The lifetime of 'aList' must exceed that of 'this'.
    ListWindow( const TRect &bounds, const char *title, const ListModel &model,
                ListViewCreator createListViewer = defListViewCreator<ListView> );

    void shutDown() override;
    void handleEvent(TEvent& event) override;
    TPalette &getPalette() const override;

    void *getCurrent();
};

class ListView : public TListViewer
{
    const ListModel &model;

public:

    // The lifetime of 'aList' must exceed that of 'this'.
    ListView( const TRect& bounds, TScrollBar *hScrollBar, TScrollBar *vScrollBar,
              const ListModel &model );

    void *getCurrent();
    virtual void getText(char *dest, short item, short maxLen) override;
    virtual void handleEvent(TEvent& ev) override;

    virtual TPalette &getPalette() const override;
};

template <class Viewer>
inline ListView *defListViewCreator(TRect r, TWindow *win, const ListModel &model)
{
    r.grow(-1, -1);
    return new Viewer( r,
                       win->standardScrollBar(sbHorizontal | sbHandleKeyboard),
                       win->standardScrollBar(sbVertical | sbHandleKeyboard),
                       model );
}

class EditorListView : public ListView
{
public:
    using ListView::ListView;

    void handleEvent(TEvent &ev) override;
};

#include "apputils.h"
#include "editwindow.h"

class EditorListModel : public ListModel
{
    mutable list_head_iterator<EditorWindow> list;

public:

    EditorListModel(list_head<EditorWindow> &aList) :
        list(&aList)
    {
    }

    size_t size() const noexcept override
    {
        return list.size();
    }

    void *at(size_t i) const noexcept override
    {
        return list.at(i)->self;
    }

    TStringView getText(void *item) const noexcept override
    {
        if (auto *wnd = (EditorWindow *) item)
            return wnd->title;
        return "";
    }
};

#endif // TURBO_LISTVIEWS_H
