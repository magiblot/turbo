#ifndef TURBO_DOCVIEW_H
#define TURBO_DOCVIEW_H

#define Uses_TSurfaceView
#include <tvision/tv.h>

#include "tscintilla.h"

struct BaseEditorWindow;

struct DocumentView : public TSurfaceView {

    Scintilla::TScintillaEditor &editor;
    BaseEditorWindow &window;

    DocumentView( const TRect &bounds,
                  const TDrawSurface *view,
                  Scintilla::TScintillaEditor &aEditor,
                  BaseEditorWindow &aWindow );

    void handleEvent(TEvent &ev) override;
    void draw() override;

    void consumeInputText(TEvent &ev);

};

#endif
