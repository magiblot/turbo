#ifndef TVEDIT_DOCTREE_H
#define TVEDIT_DOCTREE_H

#define Uses_TWindow
#define Uses_TOutline
#include <tvision/tv.h>

#include <filesystem>

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

    // The limited TOutline interface does not allow providing custom
    // parameters to seach functions, so we store them externally.
    // This doesn't support concurrent searches.

    static const void *searchArg;
    static void *searchResult;

    using TOutline::TOutline;

    void focused(int i) override;

    void addEditor(EditorWindow *w);
    void focusEditor(EditorWindow *w);
    void removeEditor(EditorWindow *w);
    DirNode *getDirNode(const std::filesystem::path &dirPath);
    static Boolean isSameDir(TOutlineViewer *, TNode *, int, int, long, ushort);
    static Boolean hasEditor(TOutlineViewer *, TNode *, int, int, long, ushort);

};

struct DocumentTreeWindow : public TWindow {

    DocumentTreeView *tree;
    DocumentTreeWindow **ptr;

    DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr);
    ~DocumentTreeWindow();

    void setState(ushort state, Boolean enable) override;
    void close() override;

};

inline void remove(TNode *parent, TNode *child)
{
    auto **indirect = &parent->childList;
    while (*indirect != child)
        indirect = &(*indirect)->next;
    *indirect = child->next;
    child->next = 0;
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
