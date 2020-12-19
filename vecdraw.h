#pragma once

#include "stdio.h"
#include "ext.h"

typedef struct {
    Point v[3];
} Triangle;

typedef struct _Vertex {
    int             ear;
    Point           v;
    struct _Vertex* prev;
    struct _Vertex* next;
} Vertex;

typedef struct {
    double mat[3][3];
} TransformMat;

typedef struct {
    unsigned char r, g, b;
} Color;

typedef struct {
    double x1, y1, x2, y2;
    double m; // yslope
} Edge;

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
    Fill      fill;
    Stroke    stroke;
    Point*    points;
    Triangle* strokes;
    Triangle* fills;
    int       n_points;
    int       n_fills;
    int       n_strokes;
    int       closed;
} Path;

typedef struct {
    int    n, capacity;
    Point* p;
} PointVector;

int isCCW(Point* p, int n);

void addVertex(Vertex** head, Point p);

Vertex* createVertexList(Point* p, int n);
void    freeVertexList(Vertex* v);
int     inCone(Vertex* a, Vertex* b);

int between(Point c, Point a, Point b);

int intersectsProper(Point a, Point b, Point c, Point d);
int intersects(Point a, Point b, Point c, Point d);

int  isDiagonal(Vertex* vertices, Vertex* a, Vertex* b);
void checkEars(Vertex* vertices);

Triangle* triangulate(Point* points, int n, int* j);
Triangle* triangulateStroke(Point* points, int N, double d, int closed, int* j);

void drawTriangle(Triangle t, TransformMat mat);

TransformMat identity();

TransformMat matMul(TransformMat m1, TransformMat m2);
TransformMat rotateOrginMat(double rad);
TransformMat translateMat(Point d);
TransformMat rotateMat(double rad, Point c);
TransformMat skewMat(double radX, double radY);
TransformMat scaleMat(Point s);
Point        transform(TransformMat m, Point p);
Triangle     transformTriangle(TransformMat m, Triangle t);

void applyTransform(TransformMat mat, Path* path);

void  applyLocalTransform(TransformMat mat, Path* path);
Path* createPath(Point* points, int n_points, Fill fill, Stroke stroke, int closed);

Path* duplicatePath(Path* from);
void  freePath(Path* path);

void strokePath(Path* path, TransformMat mat, double opacity);

void fillPath(Path* path, TransformMat mat, double opacity);

void renderPath(Path* path, TransformMat mat, double opacity);

PointVector* createPointVector();
void         pointVectorPush(PointVector* pvec, Point p);

Point pointVectorBack(PointVector* pvec);
Point pointVectorFront(PointVector* pvec);

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
    return t;
}
Triangle* triangulateStroke(Point* points, int N, double d, int closed, int* j)
{
    closed      = 1;
    Triangle* t = (Triangle*)malloc(sizeof(Triangle) * 2 * N);
    Point     p1;
    Point     p2 = points[0];
    Triangle  t1, t2;
    Vec       dr1, dr2, u1, u2, d1, d2;
    double    m, n;
    d /= 2;
    int k = 0;
    for (int i = 0; i <= N + closed; i++) {
        int oendp = 0;
        p1        = p2;
        p2        = points[(i + 1) % N];
        dr1       = dr2;
        dr2       = sub(p2, p1);
        if ((m = norm(dr2)) < 1e-5) continue;
        dr2 = mul(dr2, 1 / m);
        u1  = u2;
        u2  = normalu(dr2);
        m   = clamp(dot(u1, u2), -1, 1);
        m   = sqrt((1 - m) / (1 + m));
        n   = 1.0;
        if (fabs(m + 1) < 1e-5) n = 0, m = 1.0;
        t1.v[0] = t2.v[0];
        t1.v[1] = t2.v[1];
        t1.v[2] = t2.v[0] = oendp ? add(p1, mul(u1, d)) : add(p1, add(mul(u1, n * d), mul(dr1, m * d)));
        t2.v[1]           = reflect(t2.v[0], p1);
        t2.v[2]           = t1.v[1];
        if (i > 1) t[k++] = t1, t[k++] = t2;
    }
    *j = k;
    return t;
}

void drawTriangle(Triangle t, TransformMat mat)
{
    glBegin(GL_POLYGON);
    t = transformTriangle(mat, t);
    glVertex2f(t.v[0].x, t.v[0].y);
    glVertex2f(t.v[1].x, t.v[1].y);
    glVertex2f(t.v[2].x, t.v[2].y);
    glEnd();
}

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

Triangle transformTriangle(TransformMat m, Triangle t)
{
    for (int i = 0; i < 3; i++)
        t.v[i] = transform(m, t.v[i]);
    return t;
}

void applyLocalTransform(TransformMat mat, Path* path)
{
    for (int i = 0; i < path->n_points; i++)
        path->points[i] = transform(mat, path->points[i]);
    free(path->strokes);
    free(path->fills);
    path->strokes = triangulateStroke(path->points, path->n_points, path->stroke.width, path->closed, &path->n_strokes);
    path->fills   = triangulate(path->points, path->n_points, &path->n_fills);
}

Path* createPath(Point* points, int n_points, Fill fill, Stroke stroke, int closed)
{
    Path* path     = (Path*)malloc(sizeof(Path));
    path->fill     = fill;
    path->stroke   = stroke;
    path->points   = points;
    path->n_points = n_points;
    path->closed   = closed;
    path->strokes = triangulateStroke(path->points, path->n_points, path->stroke.width, path->closed, &path->n_strokes);
    path->fills   = triangulate(path->points, path->n_points, &path->n_fills);
    return path;
}

Path* duplicatePath(Path* from)
{
    Path* path      = (Path*)malloc(sizeof(Path));
    path->fill      = from->fill;
    path->stroke    = from->stroke;
    path->n_points  = from->n_points;
    path->n_strokes = from->n_strokes;
    path->n_fills   = from->n_fills;
    path->closed    = from->closed;
    path->points    = (Point*)malloc(sizeof(Point) * path->n_points);
    path->strokes   = (Triangle*)malloc(sizeof(Triangle) * path->n_strokes);
    path->fills     = (Triangle*)malloc(sizeof(Triangle) * path->n_fills);
    memcpy(path->points, from->points, sizeof(Point) * path->n_points);
    memcpy(path->strokes, from->strokes, sizeof(Triangle) * path->n_strokes);
    memcpy(path->fills, from->fills, sizeof(Triangle) * path->n_fills);
    return path;
}

void freePath(Path* path)
{
    free(path->points);
    free(path->fills);
    free(path->strokes);
    free(path);
}

void strokePath(Path* path, TransformMat mat, double opacity)
{
    if (path->n_points < 2) return;
    iSetColorEx(path->stroke.color.r, path->stroke.color.g, path->stroke.color.b, path->stroke.opacity * opacity);
    for (int i = 0; i < path->n_strokes; i++)
        drawTriangle(path->strokes[i], mat);
}

void fillPath(Path* path, TransformMat mat, double opacity)
{
    if (path->n_points < 3) return;
    iSetColorEx(path->fill.color.r, path->fill.color.g, path->fill.color.b, path->fill.opacity * opacity);
    for (int i = 0; i < path->n_fills; i++)
        drawTriangle(path->fills[i], mat);
}

void renderPath(Path* path, TransformMat mat, double opacity)
{
    if (path->fill.fill) fillPath(path, mat, opacity);
    if (path->stroke.width > 0.1) strokePath(path, mat, opacity);
}

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