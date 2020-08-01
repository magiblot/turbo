#ifndef TVEDIT_DOCTREE_H
#define TVEDIT_DOCTREE_H

#define Uses_TWindow
#define Uses_TOutline
#include <tvision/tv.h>

#include <filesystem>
#include <functional>

struct EditorWindow;

struct DocumentTreeView : public TOutline {

    struct DirNode : public TNode {

        DirNode *parent;
        const std::filesystem::path path;

        DirNode();
        DirNode(DirNode *parent, const std::filesystem::path &path);
        void remove(TNode *child);
        void dispose();

    };

    struct FileNode : public TNode {

        DirNode *dir;
        EditorWindow *w;

        FileNode(DirNode *dir, EditorWindow *w);
        void dispose();

    };

    typedef std::function<bool(TNode *, int)> callback_t;

    // The limited TOutline interface does not allow providing custom
    // parameters to search functions, so we must store them externally.
    // This is not thread-safe.

    static const callback_t *searchCallback;

    using TOutline::TOutline;

    void focused(int i) override;

    void addEditor(EditorWindow *w);
    void focusEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);
    void focusNext();
    void focusPrev();
    DirNode *getDirNode(const std::filesystem::path &dirPath);
    TNode *findFirst(const callback_t &cb);
    static Boolean applyCallback(TOutlineViewer *, TNode *, int, int, long, ushort);
    static callback_t hasEditor(const EditorWindow *node, int *pos=nullptr);

};

struct DocumentTreeWindow : public TWindow {

    DocumentTreeView *tree;
    DocumentTreeWindow **ptr;

    DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr);
    ~DocumentTreeWindow();

    void close() override;

};

inline void remove(TNode *parent, TNode *child)
{
    auto **indirect = &parent->childList;
    while (*indirect && *indirect != child)
        indirect = &(*indirect)->next;
    if (*indirect == child) {
        *indirect = child->next;
        child->next = 0;
    }
}

inline void putLast(TNode *parent, TNode *child)
{
    auto **indirect = &parent->childList;
    while (*indirect)
        indirect = &(*indirect)->next;
    *indirect = child;
}

inline void putFirst(TNode *parent, TNode *child)
{
    child->next = parent->childList ? parent->childList : 0;
    parent->childList = child;
}

template <class Func>
inline TNode* findChild(TNode *parent, Func &&test)
{
    auto *node = parent->childList;
    while (node) {
        if (test(node))
            return node;
        node = node->next;
    }
    return nullptr;
}

#endif
