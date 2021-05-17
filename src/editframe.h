#ifndef TURBO_EDITFRAME_H
#define TURBO_EDITFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>

struct BaseEditorWindow;

struct EditorFrame : public TFrame
{

    BaseEditorWindow *editwin;

    EditorFrame(const TRect &bounds);

    void draw() override;

    void drawIndicator();

};

#endif // TURBO_EDITFRAME_H
