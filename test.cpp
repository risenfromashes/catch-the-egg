#include "vecdraw.h"
int width = 1280, height = 720;

Path* path;

double ds = 2;
Point  p1 = {(double)width / 2, (double)height / 2}, p2 = {0, 0};

void drawEllipse(Point c, Point r, double rad)
{
    double dt = ds / min(r.x, r.y);
    int    n  = 2 * PI / dt;
    Point* p  = (Point*)malloc(sizeof(Point) * n);
    double t  = 0;
    for (int i = 0; i < n; t += dt, i++) {
        p[i].x = r.x * cos(t);
        p[i].y = r.y * sin(t);
        p[i]   = add(rotate(p[i], rad, {0, 0}), c);
    }
    iSetColor(255, 0, 0);
    iPath(p, n, 2, 1);
    free(p);
}
Interval solveQuadratic(double a, double b, double c)
{
    assert(b * b >= 4 * a * c);
    double   D = sqrt(b * b - 4 * a * c);
    Interval i;
    i.l = (-b + D) / 2 / a;
    i.r = (-b - D) / 2 / a;
    return i;
}
double getEllipseParameter(Point c, Point r, double rad, Point p)
{
    double x = ((p.y - c.y) * cos(rad) - (p.x - c.x) * sin(rad)) / r.y;
    if (x > 1.0) x = 1.0;
    if (x < -1.0) x = -1.0;
    double t[2];
    t[0] = asin(x), t[1] = PI - t[0];
    t[0] = fmod(t[0] + 2 * PI, 2 * PI);
    t[1] = fmod(t[1] + 2 * PI, 2 * PI);
    Point p_;
    for (int i = 0; i < 2; i++) {
        p_.x = r.x * cos(t[i]);
        p_.y = r.y * sin(t[i]);
        p_   = add(rotate(p_, rad, {0, 0}), c);
        if (norm(sub(p, p_)) < 1e-5) return t[i];
    }
    assert(!"Point not on ellipse");
    return 0.0;
}
void drawEllipseArc(Point c, Point r, double rad, Point p1, Point p2, int laf)
{
    static int f  = 1;
    double     dt = ds / min(r.x, r.y);
    double     t1 = getEllipseParameter(c, r, rad, p1), t2 = getEllipseParameter(c, r, rad, p2), t1_, t2_;
    if (f) {
        printf("t1: %lf, t2: %lf\n", t1, t2);
        f = 0;
    }
    t1_ = min(t1, t2), t2_ = max(t1, t2);
    laf = 2 * laf - 1;
    if (laf * (t2_ - t1_) < laf * PI)
        t1 = t2_, t2 = t1_ + 2 * PI;
    else
        t1 = t1_, t2 = t2_;
    int    n = round((t2 - t1) / dt) + 1;
    Point* p = (Point*)malloc(sizeof(Point) * n);
    double t = t1;
    for (int i = 0; i < n; t += dt, i++) {
        p[i].x = r.x * cos(t);
        p[i].y = r.y * sin(t);
        p[i]   = add(rotate(p[i], rad, {0, 0}), c);
    }
    iSetColor(0, 255, 0);
    iPath(p, n, 4, 0);
    free(p);
}
void drawArc(double rx, double ry, double deg, int laf, int sf)
{
    static int ff  = 1;
    double     rad = -deg * PI / 180;
    Point      p1_ = rotate(p1, -rad, {0, 0});
    Point      p2_ = rotate(p2, -rad, {0, 0});
    Point      p0  = sub(p2_, p1_);
    double     a = rx, b = ry;
    double     x0 = p0.x;
    double     y0 = p0.y;
    double     m, c;
    int        f = 1;
    do {
        m = -y0 * a * a / (x0 * b * b);
        c = (x0 * x0 * b * b + y0 * y0 * a * a) / (2 * x0 * b * b);
        if (f)
            f = 0;
        else {
            a += 0.1;
            b += (b / a) * 0.1;
        }
    } while ((b * b * m * m + a * a) < c * c);
    Interval sol = solveQuadratic(b * b * m * m + a * a, 2 * m * c * b * b, b * b * (c * c - a * a));
    Point    c1  = {m * sol.l + c, sol.l};
    Point    c2  = {m * sol.r + c, sol.r};
    c1           = rotate(add(c1, p1_), rad, {0, 0});
    c2           = rotate(add(c2, p1_), rad, {0, 0});
    drawEllipse(c1, {a, b}, rad);
    drawEllipse(c2, {a, b}, rad);
    if ((sf && signedArea(p1, p2, c1) >= 0) || (!sf && signedArea(p1, p2, c1) < 0))
        drawEllipseArc(c1, {a, b}, rad, p1, p2, laf);
    else
        drawEllipseArc(c2, {a, b}, rad, p1, p2, laf);
}
double deg = 0.0;
int    laf = 1;
int    sf  = 1;
void   iDraw()
{
    // static int f = 1;
    // if (f) {
    //     iClear();
    //     drawPath(path);
    //     if (iGetTime() > 0.0001) f = 0;
    // }
    // drawEllipse({width / 2, height / 2, 200, 100, PI / 6);
    iClear();
    drawArc(200, 150, deg, laf, sf);
    iSetColor(0, 0, 255);
    iCircle(p1.x, p1.y, 5);
    iCircle(p2.x, p2.y, 5);
}

void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) { p2.x = mx, p2.y = my; }
void iPassiveMouseMove(int, int) {}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    if (key == 's') sf = !sf;
    if (key == 'l') laf = !laf;
    // place your codes for other keys here
}

void iSpecialKeyboard(unsigned char key)
{
    // place your codes for other keys here
    if (key == GLUT_KEY_UP) deg += 0.5;
    if (key == GLUT_KEY_DOWN) deg -= 0.5;
}

int main()
{
    // Point* p = (Point*)malloc(sizeof(Point) * 5);
    // p[0]     = {10, 10};
    // p[1]     = {160, 110};
    // p[2]     = {10, 110};
    // p[3]     = {110, 10};
    // p[4]     = {60, 160};
    // path     = createPath(p, 5, {255, 255, 0}, {255, 255, 255}, 2, 1.0, 1);
    // printf("edge count: %d\n", path->n_edges);
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
