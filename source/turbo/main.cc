#include "app.h"

static void runTurbo(int argc, const char **argv)
{
    TurboApp app(argc, argv);
    app.run();
    app.shutDown();
}

#if !defined(_WIN32) || !defined(_MSC_VER)

int main(int argc, const char *argv[])
{
    runTurbo(argc, argv);
}

#else

// Use Turbo Vision's windows.h, which omits several unneeded or unwanted
// definitions.
#include <tvision/compat/windows/windows.h>

static const char *convertToUtf8(const wchar_t *wstr) {
    int bytes = WideCharToMultiByte(
        /*CodePage*/ CP_UTF8,
        /*dwFlags*/ 0,
        /*lpWideCharStr*/ wstr,
        /*cchWideChar*/ -1,
        /*lpMultiByteStr*/ nullptr,
        /*cbMultiByte*/ 0,
        /*lpDefaultChar*/ nullptr,
        /*lpUsedDefaultChar*/ nullptr
    );

    char *result;
    if (bytes <= 1)
    {
        result = new char[1];
        result[0] = '\0';
    }
    else
    {
        result = new char[bytes];
        WideCharToMultiByte(
            /*CodePage*/ CP_UTF8,
            /*dwFlags*/ 0,
            /*lpWideCharStr*/ wstr,
            /*cchWideChar*/ -1,
            /*lpMultiByteStr*/ result,
            /*cbMultiByte*/ bytes,
            /*lpDefaultChar*/ nullptr,
            /*lpUsedDefaultChar*/ nullptr
        );
    }

    return result;
}

int wmain(int argc, const wchar_t *wargv[])
{
    // We need to convert arguments to UTF-8 manually.
    const char **argv = new const char *[argc];
    for (int i = 0; i < argc; ++i)
        argv[i] = convertToUtf8(wargv[i]);

    runTurbo(argc, argv);

    for (int i = 0; i < argc; ++i)
        delete[] argv[i];
    delete[] argv;
}

#endif // _WIN32
