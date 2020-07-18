#ifndef TVEDIT_UTIL_H
#define TVEDIT_UTIL_H

#define Uses_TDialog
#include <tvision/tv.h>

#include <string_view>

ushort execDialog(TDialog *d, void *data);
char *strnzcpy(char *dest, std::string_view src, size_t n);

#endif
