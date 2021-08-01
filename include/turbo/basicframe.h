#ifndef TURBO_BASICFRAME_H
#define TURBO_BASICFRAME_H

#define Uses_TFrame
#include <tvision/tv.h>
#include <turbo/scintilla.h>

namespace turbo {

class BasicEditorFrame : public TFrame
{
public:

    turbo::TScintilla *scintilla {nullptr}; // Non-owning. Lifetime must exceed that of 'this'.

    BasicEditorFrame(const TRect &bounds);
    void draw() override;
    void drawIndicator();
};

} // namespace turbo

#endif // TURBO_BASICFRAME_H
