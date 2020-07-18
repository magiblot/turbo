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
