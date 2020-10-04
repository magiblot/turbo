#ifndef TURBO_UTIL_H
#define TURBO_UTIL_H

#define Uses_TDialog
#define Uses_TProgram
#define Uses_TDeskTop
#include <tvision/tv.h>

#include <string_view>
#include <unordered_map>

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

    size_t size() const {
        size_t i = 0;
        const list_head *head = next;
        while (head != this) {
            ++i;
            head = head->next;
        }
        return i;
    }

    size_t empty() const {
        return this == next;
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

#include <filesystem>
#include <functional>

namespace util
{

class u8path : public std::filesystem::path
{

    using super = std::filesystem::path;

    // We should be doing something like this:
    //     std::invoke_result<decltype(&super::u8string)(void)>::type;
    // But it doesn't work, so here is a stupid workaround.
    static constexpr auto hack = [] () { std::filesystem::path p; return p.u8string(); };
    using u8string_type = decltype(std::function {hack})::result_type;

    mutable u8string_type u8s;

    void reset_u8s() const {
        // This is not thread-safe.
        u8s = super::u8string();
    }

public:

    using super::super;
    using super::operator=;

    // Override modifiers to preserve state.

    void clear() noexcept
    { super::clear(); u8s.clear(); }

    u8path& make_preferred()
    { super::make_preferred(); reset_u8s(); return *this; }

    u8path& replace_filename(const std::filesystem::path &replacement)
    { super::replace_filename(replacement); reset_u8s(); return *this; }

    u8path& replace_extension(const std::filesystem::path &replacement = {})
    { super::replace_extension(replacement); reset_u8s(); return *this; }

    void swap(std::filesystem::path &other) noexcept
    { std::swap((super&) *this, other); reset_u8s(); }

    // Accessors.

    u8path parent_path() const
    { return u8path {super::parent_path()}; }

    u8path filename() const
    { return u8path {super::filename()}; }

    u8path extension() const
    { return u8path {super::extension()}; }

    const char *c_str() const
    {
        if (u8s.size() != super::native().size())
            reset_u8s();
        return (const char *) u8s.c_str();
    }

    operator std::string_view() const
    {
        if (u8s.size() != super::native().size())
            reset_u8s();
        return {(const char *) &u8s[0], u8s.size()};
    }

    std::string_view native() const
    { return operator std::string_view(); }

};

} // namespace util
#endif
