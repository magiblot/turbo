#include <ScintillaHeaders.h>

using namespace Scintilla;

ColourDesired Platform::Chrome()
{
    return {0x7F, 0x7F, 0x7F};
}

ColourDesired Platform::ChromeHighlight()
{
    return {0xFF, 0xFF, 0xFF};
}

const char* Platform::DefaultFont()
{
    return "";
}

int Platform::DefaultFontSize()
{
    return 1;
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
