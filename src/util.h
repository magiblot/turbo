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
