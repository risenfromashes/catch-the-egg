#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include "iGraphics.h"
#define PI            acos(-1.0)
#define window_width  1280
#define window_height 720
#define bottom_margin 50
#define total_bricks  100
#define row           5
#define col           20
#define BrickLength   65
#define BrickHeight   17
#define PowerUpWidth  50
#define PowerUpHeight 46
#define totalpowerups 25
#define EPS           2

typedef struct _Point {
    double x, y;
} Point;
typedef struct _Line {
    double a, b, c; // ax + by + c = 0
} Line;
typedef struct _Velocity {
    double dx, dy;
} Velocity;
typedef struct _Color {
    int red, green, blue;
} Color;
typedef struct _Paddle {
    double bottom, left, dx, dy;
    double side_circle_radius;
} Paddle;
typedef struct _Brick {
    double left, bottom, dx, dy;
    int    type;       // defines the design or also the type, brick maybe?
    Color  temp_color; // this is probably temporary
    int    isShown;
    int    PowerUpAssociation;
} Brick;
typedef struct _Powerup {
    int type; // which bmp, sound if caught and which activity
    // 0 extra life, 1 wide paddle, 2 faster ball speed, 3 thru ball, 4 slow ball, 5 shrinkball,  6 shrinkpaddle, 7
    // supershrinkpaddle, 8 megaball, 9 death, 10 fallingbricks
    int    index; // with which brick it is, ranges from 0 to totalbricks-1
    int    state; // 0 for not triggered yet, 1 for triggered and moving, 2 for finished(either caught or not caught)
    int    isCaught;
    double left, bottom;
} Powerup;
typedef struct _Sphere {
    Point     center;
    double    radius;
    short int isMoving, isActivated;
} Sphere;

double maxi(double, double);
double mini(double, double);
void   drawBall(Sphere*);
void   drawBrick(Brick*);
double distanceOfAPointFromALine(Line*, Point*);
Line   findStraightLinePassingTwoPoints(Point*, Point*);
double evaluateFunction(Line*, Point*);
int    doesBallHitBrick(Brick*, Sphere*, Point*);
int    doesHitBrick(Brick*, Sphere*);
// int doesBallHitPaddle(Paddle*, Sphere*, Point*);
void iPassiveMouseMove(int, int);

void mousePassiveMoveHandlerFF(int mx, int my)
{
    iPassiveMouseMove(mx, window_height - my);
    glFlush();
}
double maxi(double a, double b) { return ((a > b) ? a : b); }
double mini(double a, double b) { return ((a < b) ? a : b); }
void   drawBall(Sphere* this_ball)
{
    // iSetColor(255, 255, 255);
    // double center_x = this_ball->center.x;
    // double center_y = this_ball->center.y;
    // double radius = this_ball->radius;
    // iFilledCircle(center_x, center_y, radius);
    // double now_x = center_x - radius / sqrt(2);
    // double now_y = center_y + radius / sqrt(2);
    // for (int i = 0; i < radius; i++){
    //     iSetColor(radius/5+8*i, radius/5+8*i, radius/5+8*i);
    //     iFilledCircle(now_x, now_y, 2*radius - 2*i);
    // }
    iSetColor(120, 120, 120);
    iFilledCircle(this_ball->center.x, this_ball->center.y, this_ball->radius);
    return;
}
void drawBrick(Brick* this_brick)
{
    if (1 || this_brick->type == 1) {
        iSetColor(rand() % 256, rand() % 256, rand() % 256);
        iFilledRectangle(this_brick->left, this_brick->bottom, this_brick->dx, this_brick->dy);
    }
    return;
}
Line findStraightLinePassingTwoPoints(Point* a, Point* b)
{
    Line ans;
    ans.a = a->y - b->y;
    ans.b = b->x - a->x;
    ans.c = a->x * b->y - b->x * a->y;
    return ans;
}
double evaluateFunction(Line* l, Point* p)
{
    double ans = l->a * p->x + l->b * p->y + l->c;
    return ans;
}
double distanceOfAPointFromALine(Line* l, Point* p)
{
    double distance;
    distance    = evaluateFunction(l, p);
    double also = sqrt((l->a * l->a) + (l->b * l->b));
    distance    = distance / also;
    if (distance < 0) distance *= -1;
    return distance;
}
int doesBallHitBrick(Brick* target_brick, Sphere* this_ball, Point* previous_center)
{
    Point bottom_left  = {target_brick->left, target_brick->bottom};
    Point bottom_right = {target_brick->left + target_brick->dx, target_brick->bottom};
    Point top_left     = {target_brick->left, target_brick->bottom + target_brick->dy};
    Point top_right    = {target_brick->left + target_brick->dx, target_brick->bottom + target_brick->dy};

    Line   now_line;
    double now_distance, prev_distance;
    if (target_brick->isShown == 0) return -1;
    // from left side
    now_line      = findStraightLinePassingTwoPoints(&top_left, &bottom_left);
    now_distance  = distanceOfAPointFromALine(&now_line, &(this_ball->center)); // distance from center
    prev_distance = distanceOfAPointFromALine(&now_line, previous_center);      // previous distance from center
    if (this_ball->center.y >= bottom_left.y && this_ball->center.y <= top_left.y && now_distance <= prev_distance &&
        now_distance <= this_ball->radius + 1) {
        return 1; // indicates left collision
    }

    // from right side
    now_line      = findStraightLinePassingTwoPoints(&top_right, &bottom_right);
    now_distance  = distanceOfAPointFromALine(&now_line, &(this_ball->center)); // distance from center
    prev_distance = distanceOfAPointFromALine(&now_line, previous_center);      // previous distance from center
    if (this_ball->center.y >= bottom_right.y && this_ball->center.y <= top_right.y && now_distance <= prev_distance &&
        now_distance <= this_ball->radius + 1) {
        return 2; // indicates right collision
    }

    // from up side
    now_line      = findStraightLinePassingTwoPoints(&top_left, &top_right);
    now_distance  = distanceOfAPointFromALine(&now_line, &(this_ball->center)); // distance from center
    prev_distance = distanceOfAPointFromALine(&now_line, previous_center);      // previous distance from center
    if (this_ball->center.x >= top_left.x && this_ball->center.x <= top_right.x && now_distance <= prev_distance &&
        now_distance <= this_ball->radius + 1) {
        return 3; // indicates up collision
    }

    // from bottom side
    now_line      = findStraightLinePassingTwoPoints(&bottom_left, &bottom_right);
    now_distance  = distanceOfAPointFromALine(&now_line, &(this_ball->center)); // distance from center
    prev_distance = distanceOfAPointFromALine(&now_line, previous_center);      // previous distance from center
    if (this_ball->center.x >= bottom_left.x && this_ball->center.x <= bottom_right.x &&
        now_distance <= prev_distance && now_distance <= this_ball->radius + 1) {
        return 4; // indicates bottom collision
    }

    return -1; // no collision
}
int doesHitBrick(Brick* Brick, Sphere* Ball)
{
    if (!Brick->isShown) return 0;
    if (!(Ball->center.x - Ball->radius > 1 + Brick->left + Brick->dx ||
          Ball->center.x + Ball->radius + 1 < Brick->left)) {
        if (Ball->center.y >= 1 + Brick->bottom && Ball->center.y + 1 <= Brick->bottom + Brick->dy) { return 1; }
        return -1;
    }
    if (!(Ball->center.y - Ball->radius > 1 + Brick->bottom + Brick->dy ||
          Ball->center.y + Ball->radius + 1 < Brick->bottom)) {
        if (Ball->center.x >= 1 + Brick->left && Ball->center.x + 1 <= Brick->left + Brick->dx) { return 3; }
        return -1;
    }
    return -1;
}

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
double getTime()
{
    static double t0 = _time();
    return _time() - t0;
}

void readFileToMemory(const char* filePath, char** buffer)
{
    FILE* fp = fopen(filePath, "rb");
    if (!fp) {
        perror("Couldn't open file.");
        *buffer = NULL;
        return;
    }
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    *buffer  = (char*)malloc(size);
    fseek(fp, 0, SEEK_SET);
    fread(*buffer, size, 1, fp);
    fclose(fp);
}

// int doesBallHitPaddle(Paddle* this_paddle, Sphere* this_ball, Point* previous_center)
// {
// 	if (this_ball->center.x - this_ball->radius <= this_paddle->left + this_paddle->dx + this_paddle->side_circle_radius
// && this_ball->center.x + this_ball->radius >= this_paddle->left - this_paddle->side_circle_radius){

// 	}
// }