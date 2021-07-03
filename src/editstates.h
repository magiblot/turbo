#ifndef TURBO_EDITSTATES_H
#define TURBO_EDITSTATES_H

#include <ScintillaHeaders.h>
#include <string_view>
#include <tvision/tv.h>

namespace Scintilla {

struct TScintillaEditor;

} // namespace Scintilla

class LineNumbersWidth
{

public:

    int minWidth;
    bool enabled {false};

    LineNumbersWidth(int min) :
        minWidth(min)
    {
    }

    void toggle()
    {
        enabled ^= true;
    }

    int update(Scintilla::TScintillaEditor &editor);

private:

    int calcWidth(Scintilla::TScintillaEditor &editor);

};

class WrapState
{
    bool enabled {false};
    bool confirmedOnce {false};

public:

    bool toggle(Scintilla::TScintillaEditor &editor, bool dialog=true);
};

class Indent
{
    bool autoIndent {true};

public:

    void toggle()
    {
        autoIndent ^= true;
    }

    void autoIndentCurrentLine(Scintilla::TScintillaEditor &editor);
};

#ifdef DocumentProperties
#undef DocumentProperties
#endif

class DocumentProperties
{
    enum : uint {
        ndEOL = 0x0001,
    };

    uint notDetected;
    int eolType;

public:

    DocumentProperties()
    {
        reset();
    }

    void reset()
    {
        notDetected = ndEOL;
        eolType = SC_EOL_LF; // Default EOL type is LF.
    }

    void analyze(std::string_view text);
    void apply(Scintilla::TScintillaEditor &editor) const;

    int getEOLType() const
    {
        return eolType;
    }

};

void stripTrailingSpaces(Scintilla::TScintillaEditor &editor);
void ensureNewlineAtEnd(Scintilla::TScintillaEditor &editor, int EOLType);

#endif
