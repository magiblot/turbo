#include "app.h"

int main(int argc, const char *argv[])
{
    TurboApp app(argc, argv);
    app.run();
    app.shutDown();
}
