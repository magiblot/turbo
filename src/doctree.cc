#include "doctree.h"
#include "editwindow.h"

const void *DocumentTreeView::searchArg {nullptr};
void *DocumentTreeView::searchResult {nullptr};

using DirNode = DocumentTreeView::DirNode;
using FileNode = DocumentTreeView::FileNode;

DirNode::DirNode() :
    TNode(""),
    parent(nullptr)
{
}

DirNode::DirNode(DirNode *parent, const std::filesystem::path &p) :
    TNode(p.filename().c_str()),
    parent(parent),
    path(p)
{
}

void DirNode::remove(TNode *child)
{
    ::remove(this, child);
    if (!childList)
        dispose();
}

void DirNode::dispose() {
    // The root node should not be disposed.
    if (parent) {
        parent->remove(this);
        delete this;
    }
}

FileNode::FileNode(DirNode *dir, EditorWindow *w) :
    TNode(w->title.c_str()),
    dir(dir),
    w(w)
{
}

void FileNode::dispose() {
    dir->remove(this);
    delete this;
}

void DocumentTreeView::focused(int i)
{
    TOutline::focused(i);
    if (auto *f = dynamic_cast<FileNode *>(getNode(i)))
        f->w->focus();
}

void DocumentTreeView::addEditor(EditorWindow *w)
{
    auto *dir = w->file.empty() ? (DirNode *) root : getDirNode(w->file.parent_path());
    putLast(dir, new FileNode(dir, w));
    update();
    drawView();
}

void DocumentTreeView::focusEditor(EditorWindow *w)
{
    int i;
    searchArg = w;
    searchResult = &i;
    if (firstThat(hasEditor))
        focused(i);
    searchArg = nullptr;
    searchResult = nullptr;
    update();
    drawView();
}

void DocumentTreeView::removeEditor(EditorWindow *w)
{
    searchArg = w;
    if (auto *f = (FileNode *) firstThat(hasEditor)) {
        f->dispose();
        update();
        drawView();
    };
    searchArg = nullptr;
}

DirNode* DocumentTreeView::getDirNode(const std::filesystem::path &dirPath)
{
    // The parent of the directory we are searching for.
    DirNode *parent;
    {
        auto &&parentPath = dirPath.parent_path();
        searchArg = &parentPath;
        parent = (DirNode *) firstThat(isSameDir);
        searchArg = nullptr;
    }
    if (!parent)
        parent = (DirNode *) root;
    // The directory we are searching for.
    DirNode *dir = nullptr;
    dir = (DirNode *) findChild(parent, [&dirPath] (TNode *node) {
        DirNode *dir = dynamic_cast<DirNode *>(node);
        return dir && dirPath == dir->path;
    });
    if (!dir) {
        dir = new DirNode(parent, dirPath);
        findChild(root, [this, dir, &dirPath] (TNode *node) {
            auto *child = dynamic_cast<DirNode *>(node);
            if (child && child->path.parent_path() == dirPath) {
                ((DirNode *) root)->remove(child);
                putLast(dir, child);
            }
            return false;
        });
        putFirst(parent, dir);
    }
    return dir;
}

Boolean DocumentTreeView::isSameDir(TOutlineViewer *, TNode *node, int, int, long, ushort)
{
    auto *dir = dynamic_cast<DirNode *>(node);
    return Boolean(dir && dir->path == *(const std::filesystem::path *) searchArg);
}

Boolean DocumentTreeView::hasEditor(TOutlineViewer *, TNode *node, int, int position, long, ushort)
{
    auto *file = dynamic_cast<FileNode *>(node);
    if (file && file->w == (EditorWindow *) searchArg) {
        if (searchResult)
            *(int *) searchResult = position;
        return True;
    }
    return False;
}

DocumentTreeWindow::DocumentTreeWindow(const TRect &bounds, DocumentTreeWindow **ptr) :
    TWindowInit(&DocumentTreeWindow::initFrame),
    TWindow(bounds, "Documents", wnNoNumber),
    ptr(ptr)
{
    auto *hsb = standardScrollBar(sbHorizontal),
         *vsb = standardScrollBar(sbVertical);
    tree = new DocumentTreeView(getExtent().grow(-1, -1), hsb, vsb, new DirNode);
    tree->growMode = gfGrowHiX | gfGrowHiY;
    insert(tree);
}

DocumentTreeWindow::~DocumentTreeWindow()
{
    if (ptr)
        *ptr = nullptr;
}

void DocumentTreeWindow::setState(ushort state, Boolean enable)
{
    TWindow::setState(state, enable);
    if (state == sfFocused && !enable) {
        // Put at the end of the list so that we don't get selected
        // automatically.
        auto *owner_ = owner;
        owner_->remove(this);
        owner_->insertBefore(this, owner_->last);
    }

}

void DocumentTreeWindow::close()
{
    hide(); // This window cannot be closed.
}
