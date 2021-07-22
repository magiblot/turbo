#ifndef TURBO_UTIL_H
#define TURBO_UTIL_H

#include <utility>

namespace turbo {

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

} // namespace turbo

#endif
