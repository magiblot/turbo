#include "util.h"

// Copied from the TVEdit example application.

ushort execDialog(TDialog *d, void *data)
{
    TView *p = TProgram::application->validView(d);
    if (p) {
        if (data)
            p->setData( data );
        ushort result = TProgram::deskTop->execView(p);
        if (result != cmCancel && data)
            p->getData(data);
        TObject::destroy(p);
        return result;
    }
    return cmCancel;
}

char *strnzcpy(char *dest, std::string_view src, size_t n)
{
    if (n) {
        size_t count = std::min(n - 1, src.size());
        memcpy(dest, src.data(), count);
        dest[count] = '\0';
    }
    return dest;
}
