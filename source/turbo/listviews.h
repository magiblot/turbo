#ifndef TURBO_LISTVIEWS_H
#define TURBO_LISTVIEWS_H

#define Uses_TWindow
#define Uses_TPalette
#define Uses_TListViewer
#include <tvision/tv.h>

#include <turbo/funcview.h>

class ListModel
{
public:
    virtual size_t size() const noexcept = 0;
    virtual void *at(size_t i) const noexcept = 0;
    virtual std::string getText(void *item) const noexcept = 0;

    static size_t maxItemCStrLen(const ListModel &model) noexcept;
};

/* ---------------------------------------------------------------------- */
/*      class ListWindow                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Frame passive                                               */
/*        2 = Frame active                                                */
/*        3 = Frame icon                                                  */
/*        4 = ScrollBar page area                                         */
/*        5 = ScrollBar controls                                          */
/*        6 = ListView normal                                             */
/*        7 = ListView focused                                            */
/*        8 = ListView normal (alternative)                               */
/*        9 = ListView focused (alternative)                              */
/* ---------------------------------------------------------------------- */

enum ListViewFlags : uint8_t
{
    lvScrollBars = 0x01,
    lvSelectSingleClick = 0x02,
};

class ListView;

class ListWindow : public TWindow
{
public:

    // 'ListViewT' must be 'ListView' or a subclass of it, and its constructor
    // must have the same parameters as those in 'ListView'.
    // The lifetime of 'model' must exceed that of the returned object.
    template <class ListViewT = ListView>
    static ListWindow &create( const TRect &bounds, TStringView title,
                               const ListModel &model, ushort listViewFlags ) noexcept;

    void *getCurrent() const noexcept;
    short getCurrentIndex() const noexcept;
    void setCurrentIndex(short i) noexcept;

    void shutDown() override;
    void handleEvent(TEvent& event) override;
    TColorAttr mapColor(uchar index) override;
    void sizeLimits(TPoint &min, TPoint &max) override;

private:

    using ListViewCreator = TFuncView<ListView &( const TRect &,
                                                  TScrollBar *, TScrollBar *,
                                                  const ListModel &, ushort )>;

    ListView *listView;

    static const TPoint minSize;

    ListWindow( const TRect &bounds, TStringView title, const ListModel &model,
                ushort listViewFlags, ListViewCreator createListView ) noexcept;
};

template <class ListViewT>
inline ListWindow &ListWindow::create( const TRect &bounds, TStringView title,
                                       const ListModel &model, ushort listViewFlags ) noexcept
{
    auto creator = [] ( const TRect &bounds,
                        TScrollBar *hScrollBar, TScrollBar *vScrollBar,
                        const ListModel &model, ushort listViewFlags ) -> ListView &
    {
        return *new ListViewT(bounds, hScrollBar, vScrollBar, model, listViewFlags);
    };
    return *new ListWindow(bounds, title, model, listViewFlags, creator);
}

/* ---------------------------------------------------------------------- */
/*      class ListView                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Active                                                      */
/*        2 = Focused                                                     */
/*        3 = Active (alternative)                                        */
/*        4 = Focused (alternative)                                       */
/* ---------------------------------------------------------------------- */

class ListView : public TListViewer
{
public:

    // The lifetime of 'model' must exceed that of 'this'.
    // Takes ownership over 'hScrollBar' and 'vScrollBar'.
    ListView( const TRect& bounds, TScrollBar *hScrollBar, TScrollBar *vScrollBar,
              const ListModel &model, ushort flags );

    void *getCurrent() noexcept;

    void handleEvent(TEvent& ev) override;
    void draw() override;
    TPalette &getPalette() const override;

private:

    const ListModel &model;
    ushort flags;
};

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

    size_t size() const noexcept override;
    void *at(size_t i) const noexcept override;
    std::string getText(void *item) const noexcept override;
};

template <class T>
struct SpanListModelEntry
{
    T data;
    TStringView text;
};

template <class T>
class SpanListModel : public ListModel
{
    TSpan<const SpanListModelEntry<T>> list;

public:

    // The lifetime of 'aList' must exceed that of 'this'.
    constexpr SpanListModel(TSpan<const SpanListModelEntry<T>> aList) noexcept :
        list(aList)
    {
    }

    size_t size() const noexcept override
    {
        return list.size();
    }

    void *at(size_t i) const noexcept override
    {
        return (i < list.size()) ? (void *) &list[i] : nullptr;
    }

    std::string getText(void *item) const noexcept override
    {
        if (auto *entry = (const SpanListModelEntry<T> *) item)
            return entry->text;
        return {};
    }
};

#endif // TURBO_LISTVIEWS_H
