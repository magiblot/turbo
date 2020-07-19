#ifndef TVEDIT_UTIL_H
#define TVEDIT_UTIL_H

#define Uses_TDialog
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>

#include <string_view>

char *strnzcpy(char *dest, std::string_view src, size_t n);

template<class Func>
// 'callback' should take an ushort and a TView * and return something
// evaluable to a bool.
ushort execDialog(TDialog *d, void *data, Func &&callback)
{
    TView *p = TProgram::application->validView(d);
    if (p) {
        if (data)
            p->setData( data );
        ushort result;
        do {
            result = TProgram::deskTop->execView(p);
        } while (result != cmCancel && !callback(result, p));
        TObject::destroy(p);
        return result;
    }
    return cmCancel;
}

template<typename T>
struct list_head
{
    T *self;
    list_head *next_;
    list_head *prev_;

    list_head()
    {
       self = 0;
       next_ = prev_ = 0;
    }

    ~list_head()
    {
        erase();
    }

    list_head(T *self_)
    {
        self = self_;
        next_ = prev_ = 0;
    }

    void insert_after(list_head *other)
    {
        prev_ = other;
        next_ = other->next_;
        other->next_ = this;
        if (next_)
            next_->prev_ = this;
    }

    void erase() {
        if (next_)
            next_->prev_ = prev_;
        if (prev_)
            prev_->next_ = next_;
        next_ = prev_ = 0;
    }

    T* next() {
        if (next_)
            return next_->self;
        return 0;
    }
};

struct active_counter {
    // Counter for enumerating editors opened in the same file.
    // 'count' is only reset when the number of editors reaches zero.
    uint count {0};
    uint active {0};

    uint operator++()
    {
        ++count;
        ++active;
        return count;
    }

    void operator--()
    {
        --active;
        if (active < 1)
            count = active;
    }
};

#endif
