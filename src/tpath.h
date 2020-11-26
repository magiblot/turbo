#ifndef TURBO_TPATH_H
#define TURBO_TPATH_H

#include <tvision/tv.h>
#include <tvision/compat/dir.h>

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

    static TStringView basename(TStringView path);
    static TStringView dirname(TStringView path);
    static TStringView extname(TStringView path);
    static TStringView rootname(TStringView path);
    static TStringView drivename(TStringView path);
    static Boolean isAbsolute(TStringView path);
    static Boolean exists(const char *path);
    static TStringView resolve(char abspath[MAXPATH], TStringView path);

};

#endif // TURBO_TPATH_H
