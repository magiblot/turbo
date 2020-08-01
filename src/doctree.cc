#include "doctree.h"
#include "editwindow.h"
#include "app.h"

using DirNode = DocumentTreeView::DirNode;
using FileNode = DocumentTreeView::FileNode;
using callback_t = DocumentTreeView::callback_t;

const callback_t *DocumentTreeView::searchCallback {nullptr};


DirNode::DirNode() :
    TNode("\xC2"),
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
    if (findFirst(hasEditor(w, &i)))
        focused(i);
    drawView();
}

void DocumentTreeView::removeEditor(EditorWindow *w)
{
    if (auto *f = (FileNode *) findFirst(hasEditor(w))) {
        f->dispose();
        update();
        drawView();
    };
}

void DocumentTreeView::focusNext()
{
    findFirst([this] (auto *node, auto pos) {
        if (dynamic_cast<FileNode*>(node) && pos > foc) {
            focused(pos);
            drawView();
            return true;
        }
        return false;
    });
}

void DocumentTreeView::focusPrev()
{
    int prevPos = -1;
    findFirst([this, &prevPos] (auto *node, auto pos) {
        if (dynamic_cast<FileNode*>(node)) {
            if (pos < foc)
                prevPos = pos;
            else if (prevPos > 0) {
                focused(prevPos);
                drawView();
                return true;
            }
        }
        return false;
    });
}

DirNode* DocumentTreeView::getDirNode(const std::filesystem::path &dirPath)
{
    // The parent of the directory we are searching for.
    DirNode *parent;
    {
        auto &&parentPath = dirPath.parent_path();
        auto &&cb = [&parentPath] (auto *node, ...) {
            auto *dir = dynamic_cast<DirNode *>(node);
            return dir && dir->path == parentPath;
        };
        parent = (DirNode *) findFirst(std::move(cb));
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

TNode *DocumentTreeView::findFirst(const callback_t &cb)
{
    searchCallback = &cb;
    auto *ret = firstThat(applyCallback);
    searchCallback = nullptr;
    return ret;
}

Boolean DocumentTreeView::applyCallback(TOutlineViewer *, TNode *node, int, int position, long, ushort)
{
    return Boolean((*searchCallback)(node, position));
}

callback_t DocumentTreeView::hasEditor(const EditorWindow *w, int *pos)
{
    return [w, pos] (auto *node, auto position) {
        auto *file = dynamic_cast<FileNode *>(node);
        if (file && file->w == w) {
            if (pos)
                *pos = position;
            return true;
        }
        return false;
    };
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
    message(TVEditApp::app, evCommand, cmToggleTree, 0);
}
