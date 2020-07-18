#ifndef TVEDIT_DOCVIEW_H
#define TVEDIT_DOCVIEW_H

#include "tscintilla.h"
#include "drawview.h"

class TScrollBar;
class EditorWindow;

struct DocumentView : public TDrawableView {

    Scintilla::TScintillaEditor &editor;
    EditorWindow &window;

    DocumentView( const TRect &bounds,
                  Scintilla::TScintillaEditor &aEditor,
                  EditorWindow &aWindow );

    void handleEvent(TEvent &ev) override;
    void setState(ushort aState, Boolean enable) override;

    void doUpdate();

};

#endif
