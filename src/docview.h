#ifndef TVEDIT_DOCVIEW_H
#define TVEDIT_DOCVIEW_H

#include "tscintilla.h"
#include "drawview.h"

struct DocumentView : public TDrawableView {

    Scintilla::TScintillaEditor &editor;

    DocumentView(const TRect &bounds, Scintilla::TScintillaEditor &editor_);

    void handleEvent(TEvent &ev) override;
    void doUpdate();

};

#endif
