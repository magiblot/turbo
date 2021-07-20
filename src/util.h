#ifndef TURBO_UTIL_H
#define TURBO_UTIL_H

#define Uses_TDialog
#define Uses_TProgram
#define Uses_TDeskTop
#define Uses_TFileDialog
#include <tvision/tv.h>
#include "tpath.h"

#include <string>
#include <string_view>
#include <forward_list>
#include <unordered_map>
#include <tvision/compat/dir.h>

template<class Func>
// 'callback' should take an ushort and a TView * and return something
// evaluable to a bool.
inline ushort execDialog(TDialog *d, void *data, Func &&callback)
{
    TView *p = TProgram::application->validView(d);
    if (p) {
        if (data)
            p->setData(data);
        ushort result;
        do {
            result = TProgram::application->execView(p);
        } while (result != cmCancel && !callback(p));
        TObject::destroy(p);
        return result;
    }
    return cmCancel;
}

template<typename Func>
inline void openFileDialog( TStringView aWildCard, TStringView aTitle,
                            TStringView inputName, ushort aOptions,
                            uchar histId, Func &&callback )
{
    auto *dialog = new TFileDialog( aWildCard, aTitle,
                                    inputName, aOptions,
                                    histId );
    execDialog(dialog, nullptr, std::move(callback));
}

// inline ushort execDialog(TView *d)
// {
//     TView *p = TProgram::application->validView(d);
//     if (p) {
//         ushort result = TProgram::application->execView(p);
//         TObject::destroy(p);
//         return result;
//     }
//     return cmCancel;
// }

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

    size_t size() const {
        size_t i = 0;
        const list_head *head = next;
        while (head != this) {
            ++i;
            head = head->next;
        }
        return i;
    }

    bool empty() const {
        return this == next;
    }

    bool detached() const {
        return !next && !prev;
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

    list_head_iterator(list_head<T> *list_) :
        list(list_),
        listSize(list_->size()),
        it(-1),
        itItem(list)
    {
    }

    size_t size() const
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

namespace detail {

template <class T, size_t N, class Func>
struct forEach_
{
    static void invoke(T* const * args, Func &&func)
    {
        if (args[0]) func(*args[0]);
        forEach_<T, N - 1, Func>::invoke(&args[1], std::move(func));
    }
};

template<class T, class Func>
struct forEach_<T, 0, Func>
{
    static void invoke(T* const *, Func &&)
    {
    }
};

} // namespace detail

template <class T, size_t N, class Func>
inline void forEach(T* const (&args)[N], Func &&func)
{
    detail::forEach_<T, N, Func>::invoke(&args[0], std::move(func));
}

struct CwdGuard
{
    char *lastCwd;
    CwdGuard(const char *newCwd)
    {
        if (newCwd)
        {
            lastCwd = ::getcwd(nullptr, 0);
            int r = ::chdir(newCwd); (void) r;
        }
        else
            lastCwd = nullptr;
    }
    CwdGuard(const std::string &newCwd) :
        CwdGuard(newCwd.c_str())
    {
    }
    ~CwdGuard()
    {
        if (lastCwd)
        {
            int r = chdir(lastCwd); (void) r;
            ::free(lastCwd);
        }
    }
};

class FileCounter
{
    std::unordered_map<std::string_view, active_counter> map;
    std::forward_list<std::string> strings;
public:
    active_counter &operator[](std::string_view) noexcept;
};

#endif
