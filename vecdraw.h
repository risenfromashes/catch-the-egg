#include "ext.h"
#include "stdio.h"
#define RBTREE_DONOT_COPY_VALUES
#define RBTREE_REUSE_NODES
#include "rbtree.h"

#define MAX_WIDTH 4096

typedef struct {
    unsigned char r, g, b;
} Color;

typedef struct {
    double x1, y1, x2, y2;
    double m; // yslope
} Edge;

// compare by y cord
int EdgeCompMinY(const void* p1, const void* p2)
{
    Edge *e1 = (Edge*)p1, *e2 = (Edge*)p2;
    if (fabs(e1->y1 - e2->y1) < 1e-8)
        return 0;
    else if (e1->y1 < e2->y1)
        return -1;
    else
        return 1;
}

int EdgeCompMaxY(const void* p1, const void* p2)
{
    Edge *e1 = (Edge*)p1, *e2 = (Edge*)p2;
    if (fabs(e1->y2 - e2->y2) < 1e-8)
        return 0;
    else if (e1->y2 < e2->y2)
        return -1;
    else
        return 1;
}

typedef struct {
    unsigned char fill;
    Color         color;
    float         opacity;
} Fill;

typedef struct {
    float width;
    Color color;
    float opacity;
} Stroke;

typedef struct {
    Fill   fill;
    Stroke stroke;
    Point* points;
    int    n_points;
    int    closed;
    Edge*  edges;
    int    n_edges;
    double x_min, x_max;
    double y_min, y_max;
} Path;

Path* createPath(Point* points, int n_points, Fill fill, Stroke stroke, int closed)
{
    Path* path     = (Path*)malloc(sizeof(Path));
    path->fill     = fill;
    path->stroke   = stroke;
    path->points   = points;
    path->n_points = n_points;
    path->closed   = closed;
    path->x_min = INFINITY, path->x_max = -INFINITY;
    path->y_min = INFINITY, path->y_max = -INFINITY;
    path->edges = (Edge*)malloc(sizeof(Edge) * n_points);
    int k       = 0;
    for (int i = 0; i < path->n_points; i++) {
        Point p1 = path->points[i], p2 = path->points[(i + 1) % n_points];
        path->x_min = min(path->x_min, p1.x);
        path->x_max = max(path->x_max, p1.x);
        path->y_min = min(path->y_min, p1.y);
        path->y_max = max(path->y_max, p1.y);
        if (fabs(p1.y - p2.y) > 1e-8) {
            double m = (p1.x - p2.x) / (p1.y - p2.y);
            if (p1.y < p2.y)
                path->edges[k] = {.x1 = p1.x, .y1 = p1.y, .x2 = p2.x, .y2 = p2.y, .m = m};
            else
                path->edges[k] = {.x1 = p2.x, .y1 = p2.y, .x2 = p1.x, .y2 = p1.y, .m = m};
            k++;
        }
    }
    path->n_edges = k;
    qsort(path->edges, path->n_edges, sizeof(Edge), EdgeCompMinY);
    return path;
}

void freePath(Path* path)
{
    free(path->points);
    if (path->edges) free(path->edges);
    free(path);
}
void strokePath(Path* path)
{
    iSetColorEx(path->stroke.color.r, path->stroke.color.g, path->stroke.color.b, path->stroke.opacity);
    iPath(path->points, path->n_points, path->stroke.width, path->closed);
}

void fillPath(Path* path)
{
    iSetColorEx(path->fill.color.r, path->fill.color.g, path->fill.color.b, path->fill.opacity);
    static RBTree* activeEdges = createRBTree(EdgeCompMaxY);
    int            intersections[MAX_WIDTH + 5];
    double         width  = iScreenWidth;
    double         height = iScreenHeight;
    double         x_min = max(0, path->x_min), x_max = min(width, ceil(path->x_max));
    double         y_min = max(0, path->y_min), y_max = min(height, ceil(path->y_max));
    int            k = 0;
    for (int y = y_min; y <= y_max; y++) {
        while (k < path->n_edges && (int)round(path->edges[k].y1) <= y)
            RBTreeInsert(activeEdges, &path->edges[k++]);
        RBNode *max = RBTreeMax(activeEdges), *max_;
        while (max != RBNull && (int)round(RBValue(max, Edge).y2) < y) {
            max_ = max;
            max  = RBNodePrev(max);
            RBTreeDelete(activeEdges, max_);
        }
        for (int x = x_min, c = 0; x <= x_max; x++)
            intersections[x] = 0;
        for (RBNode* n = RBTreeMin(activeEdges); n != RBNull; n = RBNodeNext(n)) {
            Edge* e = RBPointer(n, Edge);
            int   x = round(e->x1 + e->m * (y - e->y1));
            if (x >= 0 && x <= width) intersections[x]++;
        }
        double x1, x2;
        for (int x = x_min, c = 0, c0; x <= x_max; x++)
            if (intersections[x]) {
                c0 = c;
                c += intersections[x];
                x1 = x2;
                x2 = x;
                if (c0 % 2) iLine(x1, y, x2, y);
            }
    }
    RBTreeClear(activeEdges);
}

void drawPath(Path* path)
{
    fillPath(path);
    if (path->stroke.width > 0.1) strokePath(path);
}
typedef struct {
    double mat[3][3];
} TransformMat;

TransformMat identity()
{
    TransformMat mat;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mat.mat[i][j] == i == j;
    return mat;
}

TransformMat matMul(TransformMat m1, TransformMat m2)
{
    TransformMat mat;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            mat.mat[i][j] = 0;
            for (int k = 0; k < 3; k++)
                mat.mat[i][j] += m1.mat[i][k] * m2.mat[k][j];
        }
    return mat;
}
TransformMat rotateOrginMat(double rad)
{
    TransformMat mat = identity();
    mat.mat[0][0]    = cos(rad);
    mat.mat[0][1]    = -sin(rad);
    mat.mat[1][0]    = sin(rad);
    mat.mat[1][1]    = cos(rad);
    return mat;
}
TransformMat translateMat(Point d)
{
    TransformMat mat = identity();
    mat.mat[0][2]    = d.x;
    mat.mat[1][2]    = d.y;
    return mat;
}
TransformMat rotateMat(double rad, Point c)
{
    return matMul(translateMat(c), matMul(rotateOrginMat(rad), translateMat(neg(c))));
}
TransformMat skewMat(double radX, double radY)
{
    TransformMat mat = identity();
    mat.mat[0][1]    = tan(radX);
    mat.mat[1][0]    = tan(radY);
    return mat;
}
TransformMat scaleMat(Point s)
{
    TransformMat mat = identity();
    mat.mat[0][0]    = s.x;
    mat.mat[1][1]    = s.y;
    return mat;
}

Point applyTransform(TransformMat m, Point p)
{
    return {m.mat[0][0] * p.x + m.mat[0][1] * p.y + m.mat[0][2], m.mat[1][0] * p.x + m.mat[1][1] * p.y + m.mat[1][2]};
}

typedef struct {
    int    n, capacity;
    Point* p;
} PointVector;

PointVector* createPointVector()
{
    PointVector* vec = (PointVector*)malloc(sizeof(PointVector));
    vec->n           = 0;
    vec->capacity    = 16;
    vec->p           = (Point*)malloc(sizeof(Point) * vec->capacity);
    return vec;
}

void pointVectorPush(PointVector* pvec, Point p)
{
    assert(pvec->n <= pvec->capacity);
    if (pvec->n == pvec->capacity) {
        pvec->capacity *= 2;
        Point* t = (Point*)malloc(sizeof(Point) * pvec->capacity);
        memcpy(t, pvec->p, pvec->n * sizeof(Point));
        free(pvec->p);
        pvec->p = t;
    }
    pvec->p[pvec->n++] = p;
}

Point pointVectorBack(PointVector* pvec) { return pvec->p[pvec->n - 1]; }
Point pointVectorFront(PointVector* pvec) { return pvec->p[0]; }