#include "ext.h"
#include "stdio.h"

#define RBTREE_DONOT_COPY_VALUES
#define RBTREE_REUSE_NODES

typedef struct {
    unsigned char r, g, b;
} Color;

typedef struct {
    double x1, y1, x2, y2;
    double m;
} Edge;

typedef struct _Path {
    Color  fillColor;
    Color  strokeColor;
    double strokeWidth;
    double opacity;
    Point* points;
    int    n_points;
    int    closed;
    int    convex;
    // sorted edge list only to be used for non-convex poly or
    // gradients (yet to be implemented)
    Edge* edges;
} Path;

void strokePath(Path* path)
{
    iSetColorEx(path->strokeColor.r, path->strokeColor.g, path->strokeColor.b, path->opacity);
    iPath(path->points, path->n_points, path->strokeWidth, path->closed);
}

typedef struct _EdgeList {

} EdgeList;

void fillPath(Path* path)
{
    iSetColorEx(path->fillColor.r, path->fillColor.g, path->fillColor.b, path->opacity);
    if (path->convex)
        iFilledPolygon(path->points, path->n_points);
    else {
        double y     = path->edges[0].y1;
        double y_max = path->edges[path->n_points - 1].y1;
        int    i     = 0;
        while (y < y_max) {}
    }
}