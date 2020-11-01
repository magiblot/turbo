#ifndef TURBO_DOCVIEW_H
#define TURBO_DOCVIEW_H

#define Uses_TSurfaceView
#include <tvision/tv.h>

#include "tscintilla.h"

struct EditorWindow;

struct DocumentView : public TSurfaceView {

    Scintilla::TScintillaEditor &editor;
    EditorWindow &window;

    DocumentView( const TRect &bounds,
                  const TDrawSurface *view,
                  Scintilla::TScintillaEditor &aEditor,
                  EditorWindow &aWindow );

    void handleEvent(TEvent &ev) override;
    void draw() override;

    void consumeInputText(TEvent &ev);

};

#endif
