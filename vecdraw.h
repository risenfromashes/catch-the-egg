#pragma once

#include "stdio.h"
#include "ext.h"
#define RBTREE_DONOT_COPY_VALUES
#define RBTREE_REUSE_NODES
#include "rbtree.h"

typedef struct {
    Point v[3];
} Triangle;

typedef struct _Vertex {
    int             ear;
    Point           v;
    struct _Vertex* prev;
    struct _Vertex* next;
} Vertex;

int isCCW(Point* p, int n)
{
    double a = 0.0;
    // line integral apparantly
    for (int i = 0; i < n; i++) {
        int i1 = i, i2 = (i + 1) % n;
        a += (p[i2].x - p[i1].x) * (p[i2].y + p[i1].y);
    }
    return a < 0;
}

void addVertex(Vertex** head, Point p)
{

    Vertex* v = (Vertex*)malloc(sizeof(Vertex));
    v->ear    = 0;
    v->v      = p;
    if (*head) {
        v->next       = *head;
        v->prev       = (*head)->prev;
        v->next->prev = v;
        v->prev->next = v;
    }
    else {
        *head   = v;
        v->next = v->prev = v;
    }
}

Vertex* createVertexList(Point* p, int n)
{
    Vertex* head = NULL;
    if (isCCW(p, n)) {
        for (int i = 0; i < n; i++)
            addVertex(&head, p[i]);
    }
    else {
        for (int i = n - 1; i >= 0; i--)
            addVertex(&head, p[i]);
    }
    return head;
}
void freeVertexList(Vertex* v)
{
    if (!v) return;
    Vertex *v1 = v->next, *v2;
    while (v1 != v) {
        v2 = v1;
        v1 = v1->next;
        free(v2);
    }
    free(v);
}
int inCone(Vertex* a, Vertex* b)
{
    Vertex* a0 = a->prev;
    Vertex* a1 = a->next;
    if (leftOn(a->v, a1->v, a0->v)) return left(a->v, b->v, a0->v) && left(b->v, a->v, a1->v);
    return !(leftOn(a->v, b->v, a1->v) && leftOn(b->v, a->v, a0->v));
}

int between(Point c, Point a, Point b)
{
    if (!collinear(a, b, c)) return 0;
    if (fabs(a.x - b.x) < 1e-7) inRange(c.y, a.y, b.y);
    return inRange(c.x, a.x, b.x);
}

int intersectsProper(Point a, Point b, Point c, Point d)
{
    return (signedArea(a, b, c) * signedArea(a, b, d) < 0) && (signedArea(c, d, a) * signedArea(c, d, b) < 0);
}
int intersects(Point a, Point b, Point c, Point d)
{
    if (intersectsProper(a, b, c, d)) return 1;
    return between(c, a, b) || between(d, a, b) || between(a, c, d) || between(b, c, d);
}

int isDiagonal(Vertex* vertices, Vertex* a, Vertex* b)
{
    if (!inCone(a, b) || !inCone(b, a)) return 0;
    Vertex *v1 = vertices, *v2;
    do {
        v2 = v1->next;
        if (!(v1 == a || v1 == b || v2 == a || v2 == b) && intersects(a->v, b->v, v1->v, v2->v)) return 0;
        v1 = v1->next;
    } while (v1 != vertices);
    return 1;
}
void checkEars(Vertex* vertices)
{
    Vertex *v0, *v1, *v2;
    v1 = vertices;
    do {
        v0      = v1->prev;
        v2      = v1->next;
        v1->ear = isDiagonal(vertices, v0, v2);
        v1      = v1->next;
    } while (v1 != vertices);
}

Triangle* triangulate(Point* points, int n, int* j)
{
    Vertex* vertices = createVertexList(points, n);
    Vertex *v0, *v1, *v2;
    checkEars(vertices);
    Triangle* t = (Triangle*)malloc(sizeof(Triangle) * (n - 2));
    int       k = 0;
    while (n > 3) {
        v1      = vertices;
        int ear = 0;
        do {
            if (v1->ear) {
                ear      = 1;
                v0       = v1->prev;
                v2       = v1->next;
                t[k++]   = {.v = {v0->v, v1->v, v2->v}};
                v0->ear  = isDiagonal(vertices, v0->prev, v2);
                v2->ear  = isDiagonal(vertices, v0, v2->next);
                v0->next = v2;
                v2->prev = v0;
                vertices = v2;
                n--;
                // printf("%d\n", n);
                free(v1);
                break;
            }
            v1 = v1->next;
        } while (v1 != vertices);
        if (!ear) { break; }
    }
    t[k++] = {.v = {vertices->prev->v, vertices->v, vertices->next->v}};
    *j     = k;
    freeVertexList(vertices);
    // assert(k == (n - 2));
    // printf("finished triangulating\n");
    return t;
}

void drawTriangle(Triangle t)
{
    glBegin(GL_POLYGON);
    glVertex2f(t.v[0].x, t.v[0].y);
    glVertex2f(t.v[1].x, t.v[1].y);
    glVertex2f(t.v[2].x, t.v[2].y);
    glEnd();
}

#define MAX_WIDTH 4096
typedef struct {
    double mat[3][3];
} TransformMat;

TransformMat identity()
{
    TransformMat mat;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            mat.mat[i][j] = i == j;
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

Point transform(TransformMat m, Point p)
{
    return {m.mat[0][0] * p.x + m.mat[0][1] * p.y + m.mat[0][2], m.mat[1][0] * p.x + m.mat[1][1] * p.y + m.mat[1][2]};
}
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
    Point* init_points;
    Point* points;
    int    n_points;
    int    closed;
    Edge*  edges;
    double x_min, x_max;
    double y_min, y_max;
} Path;

void applyTransform(TransformMat mat, Path* path)
{
    for (int i = 0; i < path->n_points; i++)
        path->points[i] = transform(mat, path->init_points[i]);
    path->x_min = INFINITY, path->x_max = -INFINITY;
    path->y_min = INFINITY, path->y_max = -INFINITY;
    for (int i = 0; i < path->n_points; i++) {
        Point p1 = path->points[i], p2 = path->points[(i + 1) % path->n_points];
        path->x_min = min(path->x_min, p1.x);
        path->x_max = max(path->x_max, p1.x);
        path->y_min = min(path->y_min, p1.y);
        path->y_max = max(path->y_max, p1.y);
        double m    = (p1.x - p2.x) / (p1.y - p2.y);
        if (p1.y < p2.y)
            path->edges[i] = {.x1 = p1.x, .y1 = p1.y, .x2 = p2.x, .y2 = p2.y, .m = m};
        else
            path->edges[i] = {.x1 = p2.x, .y1 = p2.y, .x2 = p1.x, .y2 = p1.y, .m = m};
    }
    qsort(path->edges, path->n_points, sizeof(Edge), EdgeCompMinY);
}

void applyLocalTransform(TransformMat mat, Path* path)
{
    for (int i = 0; i < path->n_points; i++)
        path->init_points[i] = transform(mat, path->init_points[i]);
    applyTransform(identity(), path);
}

Path* createPath(Point* points, int n_points, Fill fill, Stroke stroke, int closed)
{
    Path* path        = (Path*)malloc(sizeof(Path));
    path->fill        = fill;
    path->stroke      = stroke;
    path->init_points = points;
    path->n_points    = n_points;
    path->closed      = closed;
    path->points      = (Point*)malloc(sizeof(Point) * n_points);
    path->edges       = (Edge*)malloc(sizeof(Edge) * n_points);
    applyTransform(identity(), path);
    return path;
}

Path* duplicatePath(Path* from)
{
    Path* path        = (Path*)malloc(sizeof(Path));
    path->fill        = from->fill;
    path->stroke      = from->stroke;
    path->n_points    = from->n_points;
    path->closed      = from->closed;
    path->init_points = (Point*)malloc(sizeof(Point) * path->n_points);
    memcpy(path->init_points, from->init_points, path->n_points * sizeof(Point));
    path->points = (Point*)malloc(sizeof(Point) * path->n_points);
    path->edges  = (Edge*)malloc(sizeof(Edge) * path->n_points);
    applyTransform(identity(), path);
    return path;
}

void freePath(Path* path)
{
    free(path->init_points);
    free(path->points);
    free(path->edges);
    free(path);
}

typedef struct {
    Point tl, tr;
    Point bl, br;
} Trapezoid;

void drawTrapezoid(Trapezoid t)
{
    glBegin(GL_POLYGON);
    glVertex2f(t.bl.x, t.bl.y);
    glVertex2f(t.br.x, t.br.y);
    glVertex2f(t.tr.x, t.tr.y);
    glVertex2f(t.tl.x, t.tl.y);
    glEnd();
}

void strokePath(Path* path)
{
    if (path->n_points < 2) return;
    iSetColorEx(path->stroke.color.r, path->stroke.color.g, path->stroke.color.b, path->stroke.opacity);
    Point     p1;
    Point     p2 = path->points[0];
    Trapezoid t;
    Vec       dr1, dr2, u1, u2, d1, d2;
    double    m, n;
    double    d = path->stroke.width / 2;
    for (int i = 0; i <= path->n_points + 1; i++) {
        p1  = p2;
        p2  = path->points[(i + 1) % path->n_points];
        dr1 = dr2;
        dr2 = sub(p2, p1);
        if ((m = norm(dr2)) < 1e-5) continue;
        dr2 = mul(dr2, 1 / m);
        u1  = u2;
        u2  = normalu(dr2);
        m   = clamp(dot(u1, u2), -1, 1);
        m   = sqrt((1 - m) / (1 + m));
        n   = 1.0;
        if (fabs(m + 1) < 1e-5) n = 0, m = 1.0;
        t.tl = t.tr;
        t.bl = t.br;
        t.tr = add(p1, add(mul(u1, n * d), mul(dr1, m * d)));
        t.br = reflect(t.tr, p1);
        if (i > 1) drawTrapezoid(t);
    }
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
        while (k < path->n_points && (int)round(path->edges[k].y1) <= y)
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

void fillPath_(Path* path)
{
    iSetColorEx(path->fill.color.r, path->fill.color.g, path->fill.color.b, path->fill.opacity);
    int       k;
    Triangle* t = triangulate(path->points, path->n_points, &k);
    for (int i = 0; i < k; i++)
        drawTriangle(t[i]);
    free(t);
}

void renderPath(Path* path, TransformMat mat)
{
    static int f = 1;
    applyTransform(mat, path);
    if (path->fill.fill) fillPath_(path);
    if (path->stroke.width > 0.1) strokePath(path);
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