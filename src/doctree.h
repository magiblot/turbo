#ifndef TURBO_DOCTREE_H
#define TURBO_DOCTREE_H

#define Uses_TWindow
#define Uses_TOutline
#include <tvision/tv.h>

#include <variant>
#include <string_view>
#include "util.h"

struct EditorWindow;

struct DocumentTreeView : public TOutline {

    struct Node : public TNode {

        TNode **ptr;
        Node *parent;
        std::variant<std::string, EditorWindow *> data;

        Node(Node *parent, std::string_view path);
        Node(Node *parent, EditorWindow *w);
        bool hasEditor() const;
        EditorWindow* getEditor();
        void setParent(Node *parent);
        void remove();
        void dispose();

    };

    typedef std::function<bool(TNode *, int)> callback_t;

    bool focusing {false};

    using TOutline::TOutline;

    void focused(int i) override;

    void addEditor(EditorWindow *w);
    void focusEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);
    void focusNext();
    void focusPrev();
    Node *getDirNode(std::string_view dirPath);
    Node *findByEditor(const EditorWindow *w, int *pos=nullptr);
    Node *findByPath(std::string_view path);
    template <class Func>
    Node *firstThat(Func &&func);

};

template <class Func>
inline DocumentTreeView::Node *DocumentTreeView::firstThat(Func &&func)
{
    auto applyCallback =
    [] ( TOutlineViewer *, TNode *node, int, int position,
         long, ushort, void *arg )
    {
        return (*(Func *) arg)((Node *) node, position);
    };

    return (Node *) TOutlineViewer::firstThat(applyCallback, &func);
}

struct DocumentTreeWindow : public TWindow {

    DocumentTreeView *tree;
    DocumentTreeWindow **ptr;

    DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr);
    ~DocumentTreeWindow();

    void close() override;

};

inline void putLast(TNode **indirect, DocumentTreeView::Node *node)
{
    // Warning: if you want to change the parent of a node, use
    // setParent() instead. Otherwise, node->parent will be a dangling pointer.
    node->next = nullptr;
    while (*indirect)
        indirect = &(*indirect)->next;
    *indirect = node;
    node->ptr = indirect;
}

inline void putFirst(TNode **indirect, DocumentTreeView::Node *node)
{
    node->next = *indirect;
    if (*indirect)
        ((DocumentTreeView::Node *) *indirect)->ptr = &node->next;
    *indirect = node;
    node->ptr = indirect;
}

template <class Func>
inline TNode* findInList(TNode **list, Func &&test)
{
    auto *node = *list;
    while (node) {
        auto *next = node->next;
        if (test((DocumentTreeView::Node *) node))
            return node;
        node = next;
    }
    return nullptr;
}

#endif
