#ifndef TURBO_DOCVIEW_H
#define TURBO_DOCVIEW_H

#include "tscintilla.h"
#include "drawviews.h"

struct EditorWindow;

struct DocumentView : public TDrawSubView {

    Scintilla::TScintillaEditor &editor;
    EditorWindow &window;

    DocumentView( const TRect &bounds,
                  const TDrawableView &view,
                  Scintilla::TScintillaEditor &aEditor,
                  EditorWindow &aWindow );

    void handleEvent(TEvent &ev) override;
    void draw() override;

};

#endif
