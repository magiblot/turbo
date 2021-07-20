#ifndef TURBO_EDITFRAME_H
#define TURBO_EDITFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>

namespace Scintilla {

struct TScintillaEditor;

} // namespace Scintilla

struct EditorFrame : public TFrame
{

    Scintilla::TScintillaEditor *editor;

    EditorFrame(const TRect &bounds);

    void draw() override;

    void drawIndicator();

};

#endif // TURBO_EDITFRAME_H
