#include <ScintillaHeaders.h>

using namespace Scintilla;

ColourDesired Platform::Chrome()
{
    return {};
}

ColourDesired Platform::ChromeHighlight()
{
    return {};
}

const char* Platform::DefaultFont()
{
    return 0;
}

int Platform::DefaultFontSize()
{
    return 0;
}

unsigned int Platform::DoubleClickTime()
{
    return 0;
}

void Platform::DebugDisplay(const char *s)
{
}

void Platform::DebugPrintf(const char *format, ...)
{
}

bool Platform::ShowAssertionPopUps(bool assertionPopUps_)
{
    return false;
}

void Platform::Assert(const char *c, const char *file, int line)
{
}
