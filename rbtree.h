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

#define RBValue(n, type)   *(type*)n->value
#define RBPointer(n, type) (type*)n->value

typedef struct {
    RBNode* root;
    int (*compFunc)(void*, void*);
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
    int (*compFunc)(void*, void*));

RBNode* RBNodeMin(RBNode* z);
RBNode* RBTreeMin(RBTree* tree);
RBNode* RBNodeMax(RBNode* z);
RBNode* RBTreeMax(RBTree* tree);
RBNode* RBNodeNext(RBNode* z);
RBNode* RBNodePrev(RBNode* z);
RBNode* RBNodeFind(RBTree* tree, RBNode* z, void* key);
RBNode* RBTreeFind(RBTree* tree, void* key);
void    RBTreeRotate(RBTree* tree, RBNode* x, int left);
RBNode* RBTreeInsert(RBTree* tree, void* value);
RBNode* RBTreeReplace(RBTree* tree, RBNode* u, RBNode* v);
void    RBTreeDelete(RBTree* tree, RBNode* z);
