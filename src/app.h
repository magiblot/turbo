#ifndef TVEDIT_APP_H
#define TVEDIT_APP_H

#define Uses_TApplication
#include <tvision/tv.h>

#include <memory>

namespace tvedit {

struct TVEditApp: public TApplication {

    void newEditorWindow();

    TVEditApp();
    static TMenuBar* initMenuBar(TRect r);
    static TStatusLine* initStatusLine(TRect r);
    void handleEvent(TEvent& event) override;

};

extern TVEditApp *app;

}

#endif
