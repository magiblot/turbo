#ifndef TURBO_TPATH_H
#define TURBO_TPATH_H

#include <tvision/tv.h>
#include <tvision/compat/dir.h> // MAXPATH

class TPath
{

public:

    static constexpr Boolean isSep(char c)
    {
        return c == '/' || c == '\\';
    }

    static constexpr Boolean isDrive(char c)
    {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    }

    static TStringView basename(TStringView path) noexcept;
    static TStringView dirname(TStringView path) noexcept;
    static TStringView extname(TStringView path) noexcept;
    static TStringView rootname(TStringView path) noexcept;
    static TStringView drivename(TStringView path) noexcept;
    static Boolean isAbsolute(TStringView path) noexcept;
    static Boolean exists(const char *path) noexcept;
    static TStringView resolve(char abspath[MAXPATH], TStringView path) noexcept;

};

#endif // TURBO_TPATH_H
