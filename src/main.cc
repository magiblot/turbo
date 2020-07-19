#include "app.h"

int main()
{
    TVEditApp app;
    TVEditApp::app = &app;
    app.run();
    app.shutDown();
    TVEditApp::app = 0;
}
