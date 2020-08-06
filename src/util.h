#ifndef TURBO_UTIL_H
#define TURBO_UTIL_H

#define Uses_TDialog
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>

#include <string_view>
#include <unordered_map>

char *strnzcpy(char *dest, std::string_view src, size_t n);

template<class Func>
// 'callback' should take an ushort and a TView * and return something
// evaluable to a bool.
ushort execDialog(TDialog *d, void *data, Func &&callback)
{
    TView *p = TProgram::application->validView(d);
    if (p) {
        if (data)
            p->setData(data);
        ushort result;
        do {
            result = TProgram::deskTop->execView(p);
        } while (result != cmCancel && !callback(p));
        TObject::destroy(p);
        return result;
    }
    return cmCancel;
}

template<typename T>
struct list_head
{
    T *self;
    list_head *next;
    list_head *prev;

    list_head()
    {
       self = 0;
       next = prev = this;
    }

    ~list_head()
    {
        remove();
    }

    list_head(T *self_)
    {
        self = self_;
        next = prev = 0;
    }

    void insert_after(list_head *other)
    {
        remove();
        prev = other;
        next = other->next;
        other->next = this;
        if (next)
            next->prev = this;
    }

    void remove() {
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;
        next = prev = 0;
    }

    template<class Func>
    void forEach(Func &&callback) {
        list_head *head = next;
        while (head != this) {
            callback(head->self);
            head = head->next;
        }
    }

};

template<typename T>
class list_head_iterator
{

    list_head<T> *list;
    size_t listSize;
    intptr_t it;
    list_head<T> *itItem;

public:

    list_head_iterator(list_head<T> *list_, size_t size_) :
        list(list_),
        listSize(size_),
        it(-1),
        itItem(list)
    {
    }

    size_t size()
    {
        return listSize;
    }

    void* at(intptr_t i)
    {
        if (i > it) {
            if (i - it <= intptr_t(listSize) - i)
                return seekF(i);
            else {
                it = listSize;
                itItem = list;
                return seekB(i);
            }
        } else if (it > i) {
            if (it - i >= i) {
                it = -1;
                itItem = list;
                return seekF(i);
            } else
                return seekB(i);
        }
        return itItem;
    }

private:

    void* seekF(intptr_t i)
    {
        while (it < i) {
            itItem = itItem->next;
            ++it;
        }
        return itItem;
    }

    void* seekB(intptr_t i)
    {
        while (it > i) {
            itItem = itItem->prev;
            --it;
        }
        return itItem;
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

template<typename K, typename V>
class const_unordered_map : public std::unordered_map<K, V>
{
public:

    using super = std::unordered_map<K, V>;
    using super::super;

    V operator[](const K &key) const {
        auto it = super::find(key);
        if (it == super::end())
            return V {};
        return it->second;
    }

};

#endif
