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
    double m;
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
    Color  fillColor;
    Color  strokeColor;
    double strokeWidth;
    double opacity;
    Point* points;
    int    n_points;
    int    closed;
    Edge*  edges;
    int    n_edges;
    double x_min, x_max;
} Path;

Path* createPath(
    Point* points, int n_points, Color fillColor, Color strokeColor, double strokeWidth, double opacity, int closed)
{
    Path* path        = (Path*)malloc(sizeof(Path));
    path->fillColor   = fillColor;
    path->strokeColor = strokeColor;
    path->strokeWidth = strokeWidth;
    path->opacity     = opacity;
    path->points      = points;
    path->n_points    = n_points;
    path->closed      = closed;
    path->x_min = INFINITE, path->x_max = -INFINITY;
    path->edges = (Edge*)malloc(sizeof(Edge) * n_points);
    int k       = 0;
    for (int i = 0; i < path->n_points; i++) {
        Point p1 = path->points[i], p2 = path->points[(i + 1) % n_points];
        path->x_min = min(path->x_min, p1.x);
        path->x_max = max(path->x_max, p1.x);
        if (fabs(p1.x - p2.x) > 1e-8) {
            double m = (p1.y - p2.y) / (p1.x - p2.x);
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
    iSetColorEx(path->strokeColor.r, path->strokeColor.g, path->strokeColor.b, path->opacity);
    iPath(path->points, path->n_points, path->strokeWidth, path->closed);
}

void fillPath(Path* path)
{
    iSetColorEx(path->fillColor.r, path->fillColor.g, path->fillColor.b, path->opacity);
    static RBTree* activeEdges = createRBTree(EdgeCompMaxY);
    int            intersections[MAX_WIDTH + 5];
    double         width  = iScreenWidth;
    double         height = iScreenHeight;
    double         x_min = max(0, path->x_min), x_max = min(width, ceil(path->x_max));
    double         y_min = max(0, path->edges[0].y1), y_max = min(height, ceil(path->edges[path->n_points - 1].y2));
    int            k = 0;
    for (int y = y_min; y <= y_max; y++) {
        while (k < path->n_edges && (int)round(path->edges[k].y1) <= y)
            RBTreeInsert(activeEdges, &path->edges[k++]);
        RBNode* min = RBTreeMin(activeEdges);
        while ((int)round(RBValue(min, Edge).y2) < y) {
            min = RBNodeNext(min);
            RBTreeDelete(activeEdges, min);
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
    if (path->strokeWidth > 0.1) strokePath(path);
}