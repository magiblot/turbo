#include "util.h"

// Copied from the TVEdit example application.

char *strnzcpy(char *dest, std::string_view src, size_t n)
{
    if (n) {
        size_t count = std::min(n - 1, src.size());
        memcpy(dest, src.data(), count);
        dest[count] = '\0';
    }
    return dest;
}
