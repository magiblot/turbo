#ifndef TVEDIT_DOCVIEW_H
#define TVEDIT_DOCVIEW_H

#include "tscintilla.h"

class EditorWindow;

struct DocumentView : public TDrawSubView {

    Scintilla::TScintillaEditor &editor;
    EditorWindow &window;

    DocumentView( const TRect &bounds,
                  const TDrawableView &view,
                  Scintilla::TScintillaEditor &aEditor,
                  EditorWindow &aWindow );

    void handleEvent(TEvent &ev) override;
    void setState(ushort aState, Boolean enable) override;
    void draw() override;


};

#endif
