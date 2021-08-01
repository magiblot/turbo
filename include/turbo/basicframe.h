#ifndef TURBO_BASICFRAME_H
#define TURBO_BASICFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>
#include <turbo/scintilla.h>

namespace turbo {

class BasicEditorFrame : public TFrame
{
    TScintilla *scintilla {nullptr};

public:

    BasicEditorFrame(const TRect &bounds);
    void draw() override;
    void drawIndicator();

    // * 'aScintilla': non-owning. Lifetime must exceed that of 'this'.
    void setScintilla(TScintilla *aScintilla)
    {
        scintilla = aScintilla;
    }
};

} // namespace turbo

#endif // TURBO_BASICFRAME_H
