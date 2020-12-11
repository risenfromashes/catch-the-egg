#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

// #define RBTREE_DONOT_COPY_VALUES
// #define RBTREE_REUSE_NODES

typedef enum { RBBlack, RBRed } RBColor;

typedef struct _RBNode {
    RBColor         color;
    void*           value;
    struct _RBNode *parent, *left, *right;
} RBNode;

RBNode* _RBNull();
#define RBNull _RBNull()

#define RBValue(n, type)   (*(type*)n->value)
#define RBPointer(n, type) ((type*)n->value)

typedef struct {
    RBNode* root;
    int (*compFunc)(const void*, const void*);
#ifndef RBTREE_DONOT_COPY_VALUES
    size_t value_size;
#endif
#ifdef RBTREE_REUSE_NODES
    RBNode* recycleNodes;
#endif
} RBTree;

RBTree* createRBTree(
#ifndef RBTREE_DONOT_COPY_VALUES
    size_t value_size,
#endif
    int (*compFunc)(const void*, const void*));

RBNode* RBTreeMin(RBTree* tree);
RBNode* RBTreeMax(RBTree* tree);
RBNode* RBNodeNext(RBNode* z);
RBNode* RBNodePrev(RBNode* z);
RBNode* RBTreeFind(RBTree* tree, void* key);
RBNode* RBTreeInsert(RBTree* tree, void* value);
void    RBTreeDelete(RBTree* tree, RBNode* z);
void    RBTreeClear(RBTree* tree);
void    RBTreeFree(RBTree* tree);

static RBNode _RBNullNode;
RBNode*       _RBNull() { return &_RBNullNode; }

RBTree* createRBTree(
#ifndef RBTREE_DONOT_COPY_VALUES
    size_t value_size,
#endif
    int (*compFunc)(const void*, const void*))
{
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    tree->root   = RBNull;
#ifndef RBTREE_DONOT_COPY_VALUES
    tree->value_size = value_size;
#endif
    tree->compFunc = compFunc;
#ifdef RBTREE_REUSE_NODES
    tree->recycleNodes = NULL;
#endif
    return tree;
}

RBNode* RBNodeMin(RBNode* z)
{
    if (z->left != RBNull) return RBNodeMin(z->left);
    return z;
}

RBNode* RBTreeMin(RBTree* tree) { return RBNodeMin(tree->root); }

RBNode* RBNodeMax(RBNode* z)
{
    if (z->right != RBNull) return RBNodeMax(z->right);
    return z;
}

RBNode* RBTreeMax(RBTree* tree) { return RBNodeMax(tree->root); }

RBNode* RBNodeNext(RBNode* z)
{
    if (z->right != RBNull) return RBNodeMin(z->right);
    RBNode* y = z->parent;
    while (y != RBNull && z == y->right) {
        z = y;
        y = y->parent;
    }
    return y;
}

RBNode* RBNodePrev(RBNode* z)
{
    if (z->left != RBNull) return RBNodeMax(z->left);
    RBNode* y = z->parent;
    while (y != RBNull && z == y->left) {
        z = y;
        y = y->parent;
    }
    return y;
}
RBNode* RBNodeFind(RBTree* tree, RBNode* z, void* key)
{
    if (z == RBNull) return z;
    int comp = tree->compFunc(z->value, key);
    if (comp == 0)
        return z;
    else if (comp < 0)
        return RBNodeFind(tree, z->left, key);
    else
        return RBNodeFind(tree, z->right, key);
}

RBNode* RBTreeFind(RBTree* tree, void* key) { return RBNodeFind(tree, tree->root, key); }

void RBTreeRotate(RBTree* tree, RBNode* x, int left)
{
    RBNode* y;
    if (left) {
        y        = x->right;
        x->right = y->left;
        if (x->right != RBNull) x->right->parent = x;
        y->left = x;
    }
    else {
        y       = x->left;
        x->left = y->right;
        if (x->left != RBNull) x->left->parent = x;
        y->right = x;
    }
    if (x->parent == RBNull)
        tree->root = y;
    else if (x->parent->left == x)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->parent = x->parent;
    x->parent = y;
}

RBNode* RBTreeInsert(RBTree* tree, void* value)
{
    RBNode* z;
#ifdef RBTREE_REUSE_NODES
    if (tree->recycleNodes) {
        z                  = tree->recycleNodes;
        tree->recycleNodes = z->right;
    }
    else
        z = (RBNode*)malloc(sizeof(RBNode));
#else
    z        = (RBNode*)malloc(sizeof(RBNode));
#endif
#ifndef RBTREE_DONOT_COPY_VALUES
    z->value = malloc(tree->value_size);
    memcpy(z->value, value, tree->value_size);
#else
    z->value = value;
#endif
    z->left = z->right = RBNull;
    z->color           = RBRed;
    RBNode *x = tree->root, *y = RBNull;
    int     l;
    while (x != RBNull) {
        y        = x;
        int comp = tree->compFunc(x->value, value);
        if (comp < 0)
            x = x->left, l = 1;
        else
            x = x->right, l = 0;
    }
    z->parent = y;
    if (y == RBNull)
        tree->root = z;
    else if (l)
        y->left = z;
    else
        y->right = z;
    RBNode* ret = z;
    // fixup
    while (z->parent->color == RBRed) {
        int pLeft = z->parent == z->parent->parent->left;
        y         = pLeft ? z->parent->parent->right : z->parent->parent->left; // uncle
        if (y->color == RBRed) {
            z->parent->color = y->color = RBBlack;
            z->parent->parent->color    = RBRed;
            z                           = z->parent->parent;
        }
        else {
            if ((pLeft && z == z->parent->right) || (!pLeft && z == z->parent->left))
                z = z->parent, RBTreeRotate(tree, z, pLeft);
            z->parent->color         = RBBlack;
            z->parent->parent->color = RBRed;
            RBTreeRotate(tree, z->parent->parent, !pLeft);
        }
    }
    tree->root->color = RBBlack;
    return ret;
}

void RBTreeReplace(RBTree* tree, RBNode* u, RBNode* v)
{
    if (u->parent == RBNull)
        tree->root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    v->parent = u->parent;
}

void RBTreeDelete(RBTree* tree, RBNode* z)
{
    if (z == RBNull) return;
    RBNode *y                = z, *x;
    RBColor y_original_color = y->color;
    if (y->left == RBNull) {
        x = y->right;
        RBTreeReplace(tree, y, x);
    }
    else if (y->right == RBNull) {
        x = y->left;
        RBTreeReplace(tree, y, x);
    }
    else {
        y                = RBNodeMin(z->right);
        y_original_color = y->color;
        x                = y->right;
        if (y->parent == z)
            x->parent = y;
        else {
            RBTreeReplace(tree, y, x);
            y->right         = z->right;
            y->right->parent = y;
        }
        RBTreeReplace(tree, z, y);
        y->left         = z->left;
        y->left->parent = y;
        y->color        = z->color;
    }
    if (y_original_color == RBBlack) {
        // fixup
        int     pLeft;
        RBNode* w;
        while (x != tree->root && x->color == RBBlack) {
            pLeft = x == x->parent->left;
            w     = pLeft ? x->parent->right : x->parent->left;
            if (w->color == RBRed) {
                w->color         = RBBlack;
                w->parent->color = RBRed;
                RBTreeRotate(tree, w->parent, pLeft);
                w = pLeft ? x->parent->right : x->parent->left;
            }
            if (w->left->color == RBBlack && w->right->color == RBBlack) {
                w->color = RBRed;
                x        = x->parent;
            }
            else {
                if (pLeft && w->left->color == RBRed) {
                    w->left->color = RBBlack;
                    w->color       = RBRed;
                    RBTreeRotate(tree, w, 0); // right
                    w = x->parent->right;
                }
                if (!pLeft && w->right->color == RBRed) {
                    w->right->color = RBBlack;
                    w->color        = RBRed;
                    RBTreeRotate(tree, w, 1); // left
                    w = x->parent->left;
                }
                if (pLeft)
                    w->right->color = RBBlack;
                else
                    w->left->color = RBBlack;
                w->color         = x->parent->color;
                x->parent->color = RBBlack;
                RBTreeRotate(tree, x->parent, pLeft);
                x = tree->root;
            }
        }
        x->color = RBBlack;
    }
#ifdef RBTREE_REUSE_NODES
    z->right           = tree->recycleNodes;
    tree->recycleNodes = z;
#else
#ifndef RBTREE_DONOT_COPY_VALUES
    free(z->value);
#endif
    free(z);
#endif
}

void RBNodeClear(RBTree* tree, RBNode* z)
{
    if (z == RBNull) return;
    RBNodeClear(tree, z->left);
    RBNodeClear(tree, z->right);
    if (z == tree->root) tree->root = RBNull;
#ifdef RBTREE_REUSE_NODES
    z->right           = tree->recycleNodes;
    tree->recycleNodes = z;
#else
#ifndef RBTREE_DONOT_COPY_VALUES
    free(z->value);
#endif
    free(z);
#endif
}
void RBTreeClear(RBTree* tree) { RBNodeClear(tree, tree->root); }

void RBNodeFree(RBTree* tree, RBNode* z)
{
    if (z == RBNull) return;
    RBNodeFree(tree, z->left);
    RBNodeFree(tree, z->right);
    if (z == tree->root) tree->root = RBNull;
    free(z->value);
    free(z);
}
void RBTreeFree(RBTree* tree)
{
#ifdef RBTREE_REUSE_NODES
    RBNode *x = tree->recycleNodes, *y;
    while (x != RBNull) {
        y = x;
        x = x->right;
        free(y->value);
        free(y);
    }
#endif
    RBNodeFree(tree, tree->root);
}