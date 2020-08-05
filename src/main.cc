#include "app.h"

int main(int argc, const char *argv[])
{
    TurboApp app(argc, argv);
    TurboApp::app = &app;
    app.run();
    app.shutDown();
    TurboApp::app = 0;
}
