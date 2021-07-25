#ifndef TURBO_EDITFRAME_H
#define TURBO_EDITFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>
#include <turbo/scintilla.h>

struct EditorFrame : public TFrame
{

    turbo::TScintilla *scintilla;

    EditorFrame(const TRect &bounds);

    void draw() override;

    void drawIndicator();

};

#endif // TURBO_EDITFRAME_H
