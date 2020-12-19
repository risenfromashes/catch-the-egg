/*
 *
 * Extenstion to the iGraphics library, adding functionalities including stroked paths,
 * transparent colors, resize responsiveness.
 *
 * Ashrafur Rahman
 * October, 2020
 *
 */

#pragma once

#ifdef PI
#undef PI
#endif
// uses PI as a variable name -_-
#include "iGraphics.h"

#ifdef FREEGLUT
#include "OpenGL/include/freeglut.h"
#endif

#ifndef PI
#define PI 3.1415926535897932
#endif
#ifndef ERR
#define ERR 1e-8
#endif
#ifndef min
#define min(x, y) ((x < y) ? (x) : (y))
#endif
#ifndef max
#define max(x, y) ((x > y) ? (x) : (y))
#endif

#ifndef sgn
#define sgn(x) (2 * (x >= 0) - 1)
#endif

typedef struct {
    double x, y;
} Point;

typedef struct {
    double l, r;
} Interval;

typedef Point Vec;

void swap(void* p1, void* p2, size_t size)
{
    void* t = malloc(size);
    memcpy(t, p1, size);
    memcpy(p1, p2, size);
    memcpy(p2, t, size);
    free(t);
}

double toRad(double deg) { return deg * PI / 180; }
Point  add(Point p1, Point p2) { return {p1.x + p2.x, p1.y + p2.y}; }
Point  sub(Point p1, Point p2) { return {p1.x - p2.x, p1.y - p2.y}; }
Point  neg(Point p) { return {-p.x, -p.y}; }
Point  mul(Point p1, double f) { return {f * p1.x, f * p1.y}; }
double cross(Point p1, Point p2) { return p1.x * p2.y - p1.y * p2.x; }
double dot(Point p1, Point p2) { return p1.x * p2.x + p1.y * p2.y; }
double norm2(Point p) { return p.x * p.x + p.y * p.y; }
double norm(Point p) { return sqrt(norm2(p)); }
Point  unit(Point p) { return mul(p, 1 / norm(p)); }
Point  rotate(Point p, double t, Point c)
{
    return {(p.x - c.x) * cos(t) - (p.y - c.y) * sin(t) + c.x, (p.x - c.x) * sin(t) + (p.y - c.y) * cos(t) + c.y};
}
double angle(Point p)
{
    if (p.x >= 0 && p.y >= 0)
        return atan(p.y / p.x);
    else if (p.x < 0 && p.y >= 0)
        return PI - atan(-p.y / p.x);
    else if (p.x < 0 && p.y < 0)
        return PI + atan(p.y / p.x);
    else
        return 2 * PI - atan(-p.y / p.x);
}
Point normal(Point p)
{
    double D = norm(p);
    return {-p.y / D, p.x / D};
}
Point normalu(Point p) { return {-p.y, p.x}; }

Point reflect(Point p, Point c) { return sub(mul(c, 2), p); }

Point pmin(Point p1, Point p2) { return {min(p1.x, p2.x), min(p1.y, p2.y)}; }
Point pmax(Point p1, Point p2) { return {max(p1.x, p2.x), max(p1.y, p2.y)}; }
Vec   normmax(Vec r1, Vec r2) { return norm(r1) > norm(r2) ? r1 : r2; }
Vec   normmin(Vec r1, Vec r2) { return norm(r1) < norm(r2) ? r1 : r2; }

double clamp(double x, double l, double r)
{
    if (x < l) x = l;
    if (x > r) x = r;
    return x;
}

int inRange(double x, double l, double r) { return l <= x && x <= r || l >= x && x >= r; }
int inInterval(double x, Interval r) { return inRange(x, r.l, r.r); }
int hasOverlap(Interval a, Interval b)
{
    return inInterval(a.l, b) || inInterval(a.r, b) || inInterval(b.l, a) || inInterval(b.r, a);
}
int    inside(Interval a, Interval b) { return inInterval(a.l, b) && inInterval(a.r, b); }
double signedArea(Point p1, Point p2, Point p3)
{
    return ((p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y)) / 2;
}
int left(Point a, Point b, Point c) { return signedArea(a, b, c) > 0; }
int leftOn(Point a, Point b, Point c) { return signedArea(a, b, c) >= 0; }
int collinear(Point a, Point b, Point c) { return fabs(signedArea(a, b, c)) < 1e-7; }

Interval solveQuadratic(double a, double b, double c)
{
    assert(b * b >= 4 * a * c);
    double   D = sqrt(b * b - 4 * a * c);
    Interval i;
    i.l = (-b + D) / 2 / a;
    i.r = (-b - D) / 2 / a;
    return i;
}

static int transparent = 0;

void iSetColorEx(double r, double g, double b, double a)
{
    double mmx = 255;
    if (r > mmx) r = mmx;
    if (g > mmx) g = mmx;
    if (b > mmx) b = mmx;
    r /= mmx;
    g /= mmx;
    b /= mmx;
    glColor4f(r, g, b, a);
}

static Point solveSimul(double a1, double b1, double c1, double a2, double b2, double c2)
{
    double d = a1 * b2 - a2 * b1;
    Point  p;
    p.x = (b1 * c2 - b2 * c1) / d;
    p.y = (c1 * a2 - c2 * a1) / d;
    return p;
}

// to prevent const to non-const str conversion warning
void iText(double x, double y, const char* str, void* font = GLUT_BITMAP_8_BY_13)
{
    glRasterPos3d(x, y, 0);
    int i;
    for (i = 0; str[i]; i++) {
        glutBitmapCharacter(font, str[i]); //,GLUT_BITMAP_8_BY_13, GLUT_BITMAP_TIMES_ROMAN_24
    }
}

// overload using Point struct
void iFilledPolygon(Point p[], int n)
{
    int i;
    if (n < 3) return;
    glBegin(GL_POLYGON);
    for (i = 0; i < n; i++) {
        glVertex2f(p[i].x, p[i].y);
    }
    glEnd();
}

void iPolygon(Point Points[], int n)
{
    int i;
    if (n < 3) return;
    glBegin(GL_LINE_STRIP);
    for (i = 0; i < n; i++) {
        glVertex2f(Points[i].x, Points[i].y);
    }
    glVertex2f(Points[0].x, Points[0].y);
    glEnd();
}

// draws stroked paths
void iPath(
    Point p[], int n, double d = 1, int closed = 0, int dashed = 0, double dash = 10, double gap = 5, int aligned = 0)
{
    /*
     *  Solves equations of consecutive parallel straight lines, distanced d/2 from the
     *  original line to determine Points for the stroked path.
     *
     *  For the first end Points in non-closed, finds perpendicularly d/2 distanced Points
     *
     */

    //  p1 ------------------- p2
    //     -------------------
    //  p0 ------------------- p3
    double pX[4], pY[4];
    double dy, dx, a1, b1, c1, a2, b2, c2, M1, M2, S;
    int    s1, s2, end = n + 2 * closed;
    Point  p1, p2;
    S = 0;
    // repeating first two Points like Points in the middle for closed
    for (int i = 0; i < end; i++) {
        if (i == n - 1 && !closed) {
            s2 = s1;
            a2 = a1, b2 = b1, c2 = c1;
        }
        else {
            dy = p[(i + 1) % n].y - p[i % n].y;
            dx = p[(i + 1) % n].x - p[i % n].x;
            if (i == 0)
                s2 = 1;
            else {
                // only combination of inequalities that gives the right signs :3
                s2 = (p[i % n].x > p[(i - 1) % n].x && p[(i + 1) % n].x > p[i % n].x) ||
                     (p[i % n].x <= p[(i - 1) % n].x && p[(i + 1) % n].x <= p[i % n].x);
                s2 = s1 * (2 * s2 - 1);
            }
            if (dx > 0)
                a2 = -dy, b2 = dx;
            else
                a2 = dy, b2 = -dx;
            c2 = -(a2 * p[i % n].x + b2 * p[i % n].y);
            M2 = sqrt(a2 * a2 + b2 * b2);
        }
        // we don't want to draw the end Points normally for closed
        if ((!closed && (i == 0 || i == n - 1)) || fabs(a1 * b2 - a2 * b1) < ERR) {
            // solving the perpendicular and two parralel st lines with distance d/2 from the original
            a1 = b2, b1 = -a2, c1 = -(a1 * p[i % n].x + b1 * p[i % n].y);
            p1 = solveSimul(a1, b1, c1, a2, b2, c2 + s2 * d * M2 / 2),
            p2 = solveSimul(a1, b1, c1, a2, b2, c2 - s2 * d * M2 / 2);
        }
        // we want to draw for i == n - 1 normally for closed, but not i == 0
        else if (i != 0) {
            // solving two consecutive parallel d distanced st lines
            p1 = solveSimul(a1, b1, c1 + s1 * d * M1 / 2, a2, b2, c2 + s2 * d * M2 / 2),
            p2 = solveSimul(a1, b1, c1 - s1 * d * M1 / 2, a2, b2, c2 - s2 * d * M2 / 2);
        }
        // keeping last two Points
        pX[2] = p1.x, pY[2] = p1.y;
        pX[3] = p2.x, pY[3] = p2.y;
        // nothing to draw when i == 0 and i == 1 if closed
        if (i != 0 && !(closed && i == 1)) {
            if (dashed) {
                dx        = p[i % n].x - p[(i - 1) % n].x;
                dy        = p[i % n].y - p[(i - 1) % n].y;
                double dS = sqrt(dx * dx + dy * dy);
                double x, y;
                double S1 = S, S2;
                double tX[4], tY[4];
                // taking the vector approach for Points at perpendicular distance
                Point dr{.x = dy / dS * d / 2, .y = -dx / dS * d / 2};
                if (aligned) {
                    double t, dt;
                    int    m    = floor((dS - dash - gap) / (dash + gap));       // number of dashes in between
                    double gap_ = gap + (dS - (m + 1) * (dash + gap)) / (m + 1); // leading and trailing space
                    for (int j = -1; j <= m; j++) {
                        if (j == -1)
                            t = 0, dt = dash / 2;
                        else if (j == m)
                            t = dS - dash / 2, dt = dash / 2;
                        else {
                            t  = (dash / 2 + gap_ + j * (dash + gap_));
                            dt = dash;
                        }
                        for (int k = 0; k < 2; k++) {
                            x = (t + dt * k) / dS * dx + p[(i - 1) % n].x;
                            y = (t + dt * k) / dS * dy + p[(i - 1) % n].y;
                            if ((j == -1 && !k) || (j == m && k)) {
                                tX[1 + k] = pX[1 + k], tY[1 + k] = pY[1 + k];
                                tX[3 * k] = pX[3 * k], tY[3 * k] = pY[3 * k];
                            }
                            else {
                                tX[1 + k] = x + dr.x, tY[1 + k] = y + dr.y;
                                tX[3 * k] = x - dr.x, tY[3 * k] = y - dr.y;
                            }
                        }
                        iFilledPolygon(tX, tY, 4);
                    }
                }
                else {
                    while (S1 < dS) {
                        S2 = S1 + dash;
                        for (int j = 0; j < 2; j++) {
                            if (j == 0 && S1 <= 0) {
                                tX[1] = pX[1], tY[1] = pY[1];
                                tX[0] = pX[0], tY[0] = pY[0];
                            }
                            else if (j == 1 && S2 >= dS) {
                                tX[2] = pX[2], tY[2] = pY[2];
                                tX[3] = pX[3], tY[3] = pY[3];
                            }
                            else {
                                double t  = (j == 0 ? S1 : S2);
                                x         = t / dS * dx + p[(i - 1) % n].x;
                                y         = t / dS * dy + p[(i - 1) % n].y;
                                tX[1 + j] = x + dr.x, tY[1 + j] = y + dr.y;
                                tX[3 * j] = x - dr.x, tY[3 * j] = y - dr.y;
                            }
                        }
                        iFilledPolygon(tX, tY, 4);
                        S1 += (dash + gap);
                    }
                    if (S2 <= dS)
                        S = S1 - dS; // gap before first dash
                    else
                        S = S1 - dash - gap - dS; // unfinished dash
                }
            }
            else
                iFilledPolygon(pX, pY, 4);
        }
        a1 = a2, b1 = b2, c1 = c2, M1 = M2, s1 = s2;
        // shifting Points left
        pX[1] = pX[2], pY[1] = pY[2];
        pX[0] = pX[3], pY[0] = pY[3];
    }
}

void iRectangleEx(Point a, Point del, double d = 1, int dashed = 0, double dash = 10, double gap = 5)
{
    Point p[] = {{a.x, a.y}, {a.x + del.x, a.y}, {a.x + del.x, a.y + del.y}, {a.x, a.y + del.y}};
    iPath(p, 4, d, 1, dashed, dash, gap, 1);
}

void iCircleEx(Point c, double r, double d = 1, int dashed = 0, int slices = 100, double dash = 10, double gap = 5)
{
    Point* p = (Point*)malloc(sizeof(Point) * slices);
    double t, dt = 2 * PI / slices;
    int    i;
    for (t = i = 0; t < 2 * PI; t += dt, i++)
        p[i].x = c.x + r * cos(t), p[i].y = c.y + r * sin(t);
    iPath(p, slices, d, 1, dashed, dash, gap);
}

void iLineEx(Point p1, Point p2, double d = 1, int dashed = 0, double dash = 10, double gap = 5)
{
    Point p[] = {p1, p2};
    iPath(p, 2, d, 0, dashed, dash, gap);
}

void iSetTransparency(int state) { transparent = (state == 0) ? 0 : 1; }

static double _time()
{
    // from vulkan gettime demo
#if defined(_WIN32)
    LARGE_INTEGER freq;
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    QueryPerformanceFrequency(&freq);
    assert(freq.LowPart != 0 || freq.HighPart != 0);
    if (count.QuadPart < MAXLONGLONG / 1000000)
        assert(freq.QuadPart != 0);
    else
        assert(freq.QuadPart >= 1000000);
    return 1.0 * count.QuadPart / freq.QuadPart;
#elif defined(__linux)
    struct timespec currTime;
    clock_gettime(CLOCK_MONOTONIC, &currTime);
    return currTime.tv_sec + currTime.tv_nsec / 1e6;
#else
    return clock() / double(CLOCKS_PER_SEC);
#endif
}

double initTime = 0.0;
double iGetTime() { return _time() - initTime; }

double iRandom(double min, double max)
{
    static int f = 1;
    if (f) {
        srand((unsigned int)time(NULL));
        f = 0;
    }
    int s = 1 << 15;
    return min + (max - min) / s * (rand() % s);
}
void iHSVtoRGB(double H, double S, double V, double rgb[])
{
    double C = S * V;
    double X = C * (1 - fabs(fmod(H / 60.0, 2) - 1));
    double m = V - C;
    double r, g, b;
    if (H >= 0 && H < 60) { r = C, g = X, b = 0; }
    else if (H >= 60 && H < 120)
        r = X, g = C, b = 0;
    else if (H >= 120 && H < 180)
        r = 0, g = C, b = X;
    else if (H >= 180 && H < 240)
        r = 0, g = X, b = C;
    else if (H >= 240 && H < 300)
        r = X, g = 0, b = C;
    else
        r = C, g = 0, b = X;
    rgb[0] = (r + m) * 255;
    rgb[1] = (g + m) * 255;
    rgb[2] = (b + m) * 255;
}
void iRGBtoHSV(double R, double G, double B, double hsv[])
{
    double r    = R / 255;
    double g    = G / 255;
    double b    = B / 255;
    double Cmax = max(r, max(g, b));
    double Cmin = min(r, min(g, b));
    double del  = Cmax - Cmin;
    double h, s, v;
    if (del == 0)
        h = 0;
    else if (Cmax == r) {
        h = 60 * fmod(6.0 + (g - b) / del, 6);
    }
    else if (Cmax == g)
        h = 60 * ((b - r) / del + 2);
    else if (Cmax == b)
        h = 60 * ((r - g) / del + 4);
    s      = Cmax == 0 ? 0 : del / Cmax;
    v      = Cmax;
    hsv[0] = h, hsv[1] = s, hsv[2] = v;
}
void iRandomColor(double S, double V, double rgb[]) { iHSVtoRGB(iRandom(0, 360), S, V, rgb); }

void iPassiveMouseMove(int, int);

void mousePassiveMoveHandlerFF(int mx, int my)
{
    iPassiveMouseMove(mx, iScreenHeight - my);
    glFlush();
}

void iResize(int width, int height);

void resizeFF(int width, int height)
{
    iScreenWidth  = width;
    iScreenHeight = height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    iResize(width, height);
    glOrtho(0.0, iScreenWidth, 0.0, iScreenHeight, -1.0, 1.0);
    glViewport(0.0, 0.0, iScreenWidth, iScreenHeight);
    glutPostRedisplay();
}

void iKeyboardUp(unsigned char key);
void iSpecialKeyboardUp(unsigned char key);

void keyboardHandler3FF(unsigned char key, int x, int y)
{
    iKeyboardUp(key);
    glutPostRedisplay();
}
void keyboardHandler4FF(int key, int x, int y)
{
    iSpecialKeyboardUp(key);
    glutPostRedisplay();
}

void iExit();

void iInit()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, iScreenWidth, 0.0, iScreenHeight, -1.0, 1.0);

    glutDisplayFunc(displayFF);
    glutReshapeFunc(resizeFF);              // added resize callback
    glutKeyboardFunc(keyboardHandler1FF);   // normal
    glutSpecialFunc(keyboardHandler2FF);    // special keys
    glutKeyboardUpFunc(keyboardHandler3FF); // normal up
    glutSpecialUpFunc(keyboardHandler4FF);  // special keys up
    glutMouseFunc(mouseHandlerFF);
    glutPassiveMotionFunc(mousePassiveMoveHandlerFF); // added passive mouse move callback
    glutMotionFunc(mouseMoveHandlerFF);
    glutIdleFunc(animFF);
    glAlphaFunc(GL_GREATER, 0.0f);
    glEnable(GL_ALPHA_TEST);

    if (transparent) { // added blending mode
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_LINEAR);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_LINEAR);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_LINEAR);
}

// enables MSAA, glBlending for transparency
// also adds resize and passive mouse movement callbacks
void iInitializeEx(int width = 500, int height = 500, int gameMode = 0, const char* title = "iGraphics")
{
    initTime      = _time();
    iScreenHeight = height;
    iScreenWidth  = width;

#ifdef FREEGLUT
    int   n = 1;
    char* p[1];
    p[0] = (char*)malloc(8);
    glutInit(&n, p);
    glutSetOption(GLUT_MULTISAMPLE, 8);
#endif
#ifdef FREEGLUT
    glEnable(GLUT_MULTISAMPLE);
#endif
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_MULTISAMPLE);

    glutInitWindowSize(width, height);
    glutInitWindowPosition(10, 10);
    if (!gameMode)
        glutCreateWindow(title);
    else {
        glutGameModeString("1280x720");
        glutEnterGameMode();
        glutSetCursor(GLUT_CURSOR_NONE);
    }
    iInit();
    atexit(iExit);
    glutMainLoop();
}