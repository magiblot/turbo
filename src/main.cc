#include "app.h"

int main(int argc, const char *argv[])
{
    TVEditApp app(argc, argv);
    TVEditApp::app = &app;
    app.run();
    app.shutDown();
    TVEditApp::app = 0;
}
