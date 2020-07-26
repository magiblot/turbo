#ifndef TVEDIT_LINENUMBERS_H
#define TVEDIT_LINENUMBERS_H

#include <ScintillaHeaders.h>
#include <tuple>

class LineNumbersWidth {

    const int minWidth;
    int lastWidth {0};

public:

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    std::tuple<int, int> update(Scintilla::TScintillaEditor &editor)
    {
        size_t newLines = editor.WndProc(SCI_GETLINECOUNT, 0U, 0U);
        int newWidth = 1;
        while (newLines /= 10)
            ++newWidth;
        if (newWidth < minWidth)
            newWidth = minWidth;
        if (newWidth != lastWidth) {
            int delta = newWidth - lastWidth;
            lastWidth = newWidth;
            return {lastWidth, delta};
        }
        return {lastWidth, 0};
    }

};

#endif
