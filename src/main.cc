#include "app.h"

using namespace tvedit;

int main()
{
    TVEditApp app;
    ::app = &app;
    app.run();
    app.shutDown();
    ::app = 0;
}