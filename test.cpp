#include "ext.h"
int width = 1280, height = 720;

// Vec* Rope;
// int    n_Points    = 20;
// double Rope_width = 1000;

double g = 9.8 * 2;
typedef struct {
    double k_s, k_b, k_d, dm, dp, maxp, minp;
    Vec *  r, *r0, *rc, *a;
    char*  colliding;
    int    n;
} Rope;
typedef struct {
    Point bl, tr;
} AABB;

typedef struct {
    int l, r;
} IntPair;

void resetAABB(AABB* a) { a->bl = {INFINITY, INFINITY}, a->tr = {-INFINITY, -INFINITY}; }
int  insideAABB(AABB a, Point p) { return (a.bl.x <= p.x && p.x <= a.tr.x && a.bl.y <= p.y && p.y <= a.tr.y); }
int  inRange(double x, double l, double r) { return l <= x && x <= r || l >= x && x >= r; }
int  segmentInsideAABB(AABB a, Point p1, Point p2)
{
    if (insideAABB(a, p2) || insideAABB(a, p1)) return 1;
    if ((p1.x < a.bl.x && p2.x < a.bl.x) || (p1.y < a.bl.y && p2.y < a.bl.y) //
        || (p1.x > a.tr.x && p2.x > a.tr.x) || (p1.y > a.tr.y && p2.y > a.tr.y))
        return 0;
    Point  c1 = {a.bl.x, a.tr.y};
    Point  c2 = {a.tr.x, a.bl.y};
    double d1 = signedArea(p1, p2, a.bl) >= 0;
    double d2 = signedArea(p1, p2, c1) >= 0;
    double d3 = signedArea(p1, p2, c2) >= 0;
    double d4 = signedArea(p1, p2, a.tr) >= 0;
    return !(d1 == d2 && d2 == d3 && d3 == d4);
}
typedef struct {
    Vec*    r;
    int     n;
    double* angles;
    Vec     F;
    double  T;
    Vec     c, c0;
    double  th, th0;
    double  M, I;
    double  area;
    AABB    aabb;
} Collider;

typedef struct {
    int    isCollision;
    Vec    dr;
    double dw;
} RopeCollisionResult;

int inAngleRange(double x, double l, double r, int sgn)
{
    if (sgn * r < sgn * l) r += sgn * 2 * PI;
    if (sgn * x < sgn * l) x += sgn * 2 * PI;
    return sgn * l <= sgn * x && sgn * x < sgn * r;
}

IntPair findVertRange(Collider* cl, Point p)
{
    int    l = 0, r = cl->n - 1;
    int    ll, rr;
    double ang = angle(sub(p, cl->c));
    double s   = 2 * ((cl->angles[1] > cl->angles[0]) ^ (fabs(cl->angles[1] - cl->angles[0]) > PI)) - 1;
    if (inAngleRange(ang, cl->angles[l], cl->angles[r], s)) {
        do {
            int m = (l + r) / 2;
            if (inAngleRange(ang, cl->angles[l], cl->angles[m], s))
                r = m;
            else
                l = m;
        } while (l < r - 1);
        ll = s > 0 ? l : r;
        rr = s > 0 ? r : l;
    }
    else
        ll = r, rr = l;
    return {ll, rr};
}
// checks if the line formed by p1, p2, intersects line formed by q1,q2
// 0 doesn't intersect (both p1, p2 on the same side of q1,q2)
// 1 if intersection near q1
// 2 if intersection near q2
// 3 if intersection between q1, q2
int segmentIntersects(Point p1, Point p2, Point q1, Point q2)
{
    double d1 = signedArea(p1, p2, q1);
    double d2 = signedArea(p1, p2, q2);
    double d3 = signedArea(q1, q2, p1);
    double d4 = signedArea(q1, q2, p2);
    if (d3 * d4 > 0) return 0;
    if (d1 * d2 <= 0) return 3;
    if (fabs(d1) < fabs(d2))
        return 1;
    else
        return 2;
}
int polygonIntersects(Point* q, int n, Point p1, Point p2)
{
    // ref: https: // stackoverflow.com/questions/4497841
    int l = 0, r = n, m;
    do {
        m     = (l + r) / 2;
        int d = segmentIntersects(p1, p2, q[l], q[m]);
        if (d == 3) return 1;
        if (d == 0) {
            int d1 = signedArea(q[l], q[m], q[l + 1]);
            int d2 = signedArea(q[l], q[m], p1);
            if (d1 * d2 > 0)
                r = m;
            else
                l = m;
        }
        else if (d == 1) {
            d = segmentIntersects(p1, p2, q[l], q[l + 1]);
            if (d == 3) return 1;
            if (d == 0) return 0;
            if (d == 1)
                l = m;
            else
                r = m;
        }
        else if (d == 2) {
            d = segmentIntersects(p1, p2, q[m], q[(m + 1) % n]);
            if (d == 3) return 1;
            if (d == 0) return 0;
            if (d == 1)
                r = m;
            else
                l = m;
        }

    } while (l < r - 1);
    return segmentIntersects(p1, p2, q[l], q[r % n]) == 3;
}
double supportFunction(Point* r, int n, Vec dir)
{
    int    i  = 0;
    int    s  = 1;
    double v  = dot(dir, r[i]);
    double v1 = dot(dir, r[(i + s) % n]);
    if (v1 < v) s = -1;
    do {
        v1 = v;
        i  = (n + i + s) % n;
        v  = dot(dir, r[i]);
    } while (v > v1);
    return v1;
}
void satisfyConstraints(Rope* c, int i)
{
    Vec    dr  = sub(c->r[i + 1], c->r[i]);
    double dr_ = norm(dr);
    if (c->minp <= dr_ && dr_ <= c->maxp) return;
    double delta;
    delta     = dr_ - (dr_ < c->minp ? c->minp : c->maxp);
    int first = i == 0, last = i == c->n - 1;
    c->r[i]     = add(c->r[i], mul(dr, delta / dr_ * (0.5 - 0.5 * first + 0.5 * last)));
    c->r[i + 1] = sub(c->r[i + 1], mul(dr, delta / dr_ * (0.5 + 0.5 * first - 0.5 * last)));
}
int freeze = 0;
int resolveCollision(Collider* cl, Rope* rp, double restitution)
{
    static int CONSTR_ITR = 4;
    int        colstart   = -1, colend;
    double     pd         = -INFINITY;
    Vec        dr;
    int        iu = 0, id = 0;
    int        collision = 0;
    iSetColor(255, 255, 0);
    iPath(rp->r, rp->n + 1, 1);
    for (int i = 0; i < rp->n; i++) {
        if (segmentInsideAABB(cl->aabb, rp->r[i], rp->r[i + 1]) &&
            polygonIntersects(cl->r, cl->n, rp->r[i], rp->r[i + 1])) {
            if (colstart < 0) colstart = i;
            if (!collision) collision = 1;
            iSetColor(0, 255, 0);
            iCircle(rp->r[i].x, rp->r[i].y, 2);
        }
        else if (colstart >= 0) {
            int k = 0;
            for (int j = colstart; j < i; j++) {
                Vec    dir = normal(sub(rp->r[j + 1], rp->r[j]));
                double d1  = fabs(supportFunction(cl->r, cl->n, dir) +
                                 supportFunction(rp->r + colstart, i - colstart + 1, neg(dir)));
                double d2  = fabs(supportFunction(cl->r, cl->n, neg(dir)) +
                                 supportFunction(rp->r + colstart, i - colstart + 1, dir));
                double pd_ = d1;
                if (d2 < d1) dir = neg(dir), pd_ = d2;
                if (pd_ > pd) {
                    k  = j;
                    pd = pd_;
                    dr = mul(dir, pd);
                }
            }
            Point p = rp->r[k];
            iSetColor(255, 0, 0);
            iCircle(p.x, p.y, 5);
            iLineEx(p, sub(p, dr), 2);
            iSetColor(255, 255, 255);
            double beta = 0.25;
            double slop = 1;
            dr          = mul(dr, (1 + beta * (2 * (norm(dr) > slop) - 1)));
            cl->c       = sub(cl->c, mul(dr, restitution));
            for (int j = 0; j < rp->n; j++)
                rp->r[j] = add(rp->r[j], mul(dr, (1 - restitution)));
            for (int k = 0; k < CONSTR_ITR; k++) {
                for (int i = 0; i < rp->n; i++)
                    satisfyConstraints(rp, i);
            }
            // freeze   = 1;
            colstart = -1;
            pd       = -INFINITY;
        }
    }
    return collision;
}

void updateCollider(Collider* c, double dt)
{
    // gravity
    double C_d = 0.000001;
    Vec    a   = mul(c->F, 1 / c->M);
    double al  = c->T / c->I;
    Vec    dr  = add(sub(c->c, c->c0), mul(a, dt * dt));
    Vec    v   = add(mul(dr, 1 / dt), mul(a, dt / 2));
    double dth = c->th - c->th0 + al * dt * dt;
    c->c0      = c->c;
    c->c       = add(c->c, dr);
    c->th0     = c->th;
    c->th += dth;
    resetAABB(&c->aabb);
    for (int i = 0; i < c->n; i++) {
        c->r[i]      = add(rotate(c->r[i], dth, c->c), dr);
        c->angles[i] = fmod(c->angles[i] + dth, 2 * PI);
        c->aabb.bl   = pmin(c->aabb.bl, c->r[i]);
        c->aabb.tr   = pmax(c->aabb.tr, c->r[i]);
    }
    c->F = {0, 0}, c->T = 0;
    Vec F_g = {0, -c->M * g};
    Vec F_d = mul(v, -C_d * c->area * norm(v));
    c->F    = add(F_g, F_d);
}

void applyForce(Collider* c, Vec F, Point r)
{
    c->F = add(c->F, F);
    c->T += cross(sub(r, c->c), F);
}

Collider* createCollider()
{
    Collider* c = (Collider*)malloc(sizeof(Collider));
    c->F        = {0, 0};
    c->T        = 0;
    c->th       = 0.001;
    c->th0      = 0.0;
    c->n        = 4;
    c->r        = (Vec*)malloc(sizeof(Vec) * c->n);
    c->angles   = (double*)malloc(sizeof(double) * c->n);
    c->r[0]     = {620, 680};
    c->r[1]     = {660, 680};
    c->r[2]     = {660, 720};
    c->r[3]     = {620, 720};
    c->M        = 2;
    c->area     = 0.0;
    c->I        = 0.0;
    resetAABB(&c->aabb);
    for (int i = 0; i < c->n; i++) {
        c->aabb.bl = pmin(c->aabb.bl, c->r[i]);
        c->aabb.tr = pmax(c->aabb.tr, c->r[i]);
        int    i1 = i, i2 = (i + 1) % c->n;
        double dA = c->r[i1].x * c->r[i2].y - c->r[i2].x * c->r[i1].y;
        c->area += 0.5 * dA;
        c->c = add(c->c, mul(add(c->r[i1], c->r[i2]), dA / 6));
        // calculate moment of inertia
        // https: // math.stackexchange.com/questions/59470/calculating-moment-of-inertia-in-2d-planar-polygon
        c->I += c->M / 12 *
                (c->r[i1].x * c->r[i1].x + c->r[i1].y * c->r[i1].y + c->r[i1].x * c->r[i2].x + c->r[i1].y * c->r[i2].y +
                 c->r[i2].x * c->r[i2].x + c->r[i2].y * c->r[i2].y) *
                dA;
    }
    c->c0 = c->c = mul(c->c, 1 / c->area);
    c->I /= c->area;
    c->I -= c->M * norm2(c->c);
    for (int i = 0; i < c->n; i++) {
        Vec    d = sub(c->r[i], c->c);
        double t = c->angles[i] = angle(d);
    }
    return c;
}

Rope*     cbl;
Collider* box;
Rope*     createRope(Vec p1, Vec p2, double Y, double A, double u, double length, int n_Points)
{
    Rope* c = (Rope*)malloc(sizeof(Rope));
    if (length < 0) length = norm(sub(p1, p2));
    c->dm = u, c->n = n_Points + 1, c->dp = length / c->n, c->k_s = 1;
    c->r         = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->r0        = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->rc        = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->a         = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->colliding = (char*)malloc(c->n + 1);
    Vec del      = mul(sub(p2, p1), 1.0 / c->n);
    c->r0[0] = c->r[0] = p1;
    for (int i = 1; i < c->n; i++)
        c->r0[i] = c->r[i] = add(c->r[i - 1], del);
    c->r0[c->n] = c->r[c->n] = p2;
    for (int i = 0; i <= c->n; i++)
        c->a[i] = {0, 0};
    return c;
}
Vec solveQuadratic(double a, double b, double c)
{
    double D = sqrt(b * b - 4 * a * c);
    return {(-b + D) / (2 * a), (-b - D) / (2 * a)};
}

double measurePotential(Rope* c, int i)
{
    Vec    dr  = sub(c->r[i + 1], c->r[i]);
    double U_k = 0.5 * c->k_s * 10 * pow(c->dp - norm(dr), 2);
    double U_g = 0.5 * c->dm * (c->r[i + 1].y + c->r[i].y) * g;
    return U_k + U_g;
}
void updateRope(Rope* c, double dt)
{
    // mass-spring model
    // verlet integration with constraints
    static int CONSTR_ITR = 4;
    c->dm                 = 0.001;
    c->k_s                = 500;
    c->k_b                = 10000;
    c->k_d                = 0.0001;
    double minst          = 0.999;
    double maxst          = 1.001;
    c->minp               = minst * c->dp;
    c->maxp               = maxst * c->dp;
    static int f1 = 1, f2 = 1;
    for (int i = 1; i < c->n; i++) {
        Vec temp = c->r[i];
        c->r[i]  = add(sub(mul(c->r[i], 2), c->r0[i]), mul(c->a[i], dt * dt));
        c->r0[i] = temp;
    }
    double U = 0.0, U_ = 0.0;
    // satisfy constraints
    for (int k = 0; k < CONSTR_ITR; k++) {
        for (int i = 0; i < c->n; i++) {
            satisfyConstraints(c, i);
            // record internal energy at collision less state
        }
    }
    // collision resolution
    resolveCollision(box, c, 0.95);

    for (int i = 1; i < c->n; i++) {
        Vec    r = c->r[i], r1 = c->r[i - 1], r2 = c->r[i + 1];
        Vec    v   = add(mul(sub(c->r[i], c->r0[i]), 1 / dt), mul(c->a[i], dt / 2));
        Vec    dr1 = sub(r1, r), dr2 = sub(r2, r);
        double dr1_ = norm(dr1), dr2_ = norm(dr2);
        /// stretch strain
        Vec F_s = add(mul(dr1, c->k_s * (1 - c->dp / dr1_)), mul(dr2, c->k_s * (1 - c->dp / dr2_)));
        //  bend strain
        double cos_ = dot(neg(dr1), dr2) / (dr1_ * dr2_);
        cos_        = cos_ < -1 ? -1 : (cos_ > 1 ? 1 : cos_);
        Vec F_b     = mul(add(mul(sub(mul(r, 2), add(r1, r2)), dr1_ * dr2_),
                          mul(add(mul(dr1, dr2_ / dr1_), mul(dr2, dr1_ / dr2_)), dot(dr1, dr2))),
                      -c->k_b / (dr1_ * dr1_ * dr2_ * dr2_) * acos(cos_));
        //  gravity
        Vec F_g = {0, -g * c->dm};
        // drag
        Vec F_d = mul(v, -norm(v) * c->k_d);
        Vec a   = mul(add(F_s, add(F_b, add(F_g, F_d))), 1 / c->dm);
        c->a[i] = a;
    }
}

void updateWorld()
{
    static double t0 = iGetTime();
    double        t  = iGetTime();
    double        Dt = (t - t0) * 10;
    t0               = t;
    double dt        = 0.002;
    double N         = Dt / dt;
    for (int i = 0; i < N; i++) {
        updateCollider(box, dt);
        updateRope(cbl, dt);
        if (freeze) break;
    }
}

void init()
{
    cbl = createRope({40, 200}, {1240, 200}, 2e9, 5e-5, 5.75e-8, -1, 200);
    box = createCollider();
}

void iDraw()
{
    if (!freeze) {
        iClear();
        iSetColor(255, 255, 255);
        iPath(cbl->r, cbl->n + 1, 4, 0);
        iSetColor(0, 0, 255);
        iPath(box->r, box->n, 1, 1);
        updateWorld();
    }
}

void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {}
void iPassiveMouseMove(int, int) {}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    freeze = 0;
    // place your codes for other keys here
}

void iSpecialKeyboard(unsigned char key)
{
    // place your codes for other keys here
}

int main()
{
    init();
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
