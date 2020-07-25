#ifndef TVEDIT_LISTVIEWS_H
#define TVEDIT_LISTVIEWS_H

#define Uses_TWindow
#define Uses_TPalette
#define Uses_TListViewer
#include <tvision/tv.h>

#include <string_view>
#include <functional>

class ListView;

class ListWindow : public TWindow
{

public:

    class List {
    public:

        virtual size_t size() = 0;
        virtual void* at(size_t i) = 0;
        virtual std::string_view getText(void *) = 0;

        virtual size_t measureWidth()
        {
            size_t width = 0, elems = size();
            for (size_t i = 0; i < elems; ++i) {
                size_t w = cstrlen(getText(at(i)));
                if (w > width)
                    width = w;
            }
            return width;
        }

    };

    ListWindow( const TRect &bounds, const char *aTitle, ListWindow::List &aList );
    ListWindow( const TRect &bounds, const char *aTitle, ListWindow::List &aList,
                std::function<ListView *(TRect, TWindow *, List &)> &&cListViewer );

    void* getSelected();
    void handleEvent(TEvent& event) override;
    virtual TPalette& getPalette() const override;

protected:

    ListView *viewer;
    List &list;

private:

    static TPalette palette;

};

class ListView : public TListViewer
{

public:

    ListView( const TRect& bounds,
              TScrollBar *aHScrollBar,
              TScrollBar *aVScrollBar,
              ListWindow::List &aList );

    virtual void* getSelected();
    virtual void getText(char *dest, short item, short maxLen) override;
    virtual void handleEvent(TEvent& ev) override;

    virtual TPalette& getPalette() const;

protected:

    ListWindow::List &list;

private:

    static TPalette palette;

};

template <class Viewer>
ListView* initViewer(TRect r, TWindow *win, ListWindow::List &list)
{
    r.grow(-1, -1);
    return new Viewer( r,
                       win->standardScrollBar(sbHorizontal | sbHandleKeyboard),
                       win->standardScrollBar(sbVertical | sbHandleKeyboard),
                       list );
}

class EditorListView : public ListView
{

public:

    using ListView::ListView;

    int disableWrap {0};

    void focusItemNum(short item) override;
    void handleEvent(TEvent &ev) override;

};

#include "util.h"
#include "editwindow.h"

class EditorList : public ListWindow::List, public list_head_iterator<EditorWindow>
{

public:

    using list_head_iterator::list_head_iterator;

    size_t size() override
    {
        return list_head_iterator::size();
    }

    void* at(size_t i) override
    {
        return list_head_iterator::at(i);
    }

    std::string_view getText(void *head_) override
    {
        auto *head = (list_head<EditorWindow> *) head_;
        return head->self->title;
    }

};

#endif
