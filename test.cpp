#include "vecdraw.h"
int width = 1280, height = 720;

Path* path;
void  iDraw()
{
    static int f = 1;
    if (f) {
        iClear();
        drawPath(path);
        if (iGetTime() > 0.0001) f = 0;
    }
}

void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {}
void iPassiveMouseMove(int, int) {}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    // place your codes for other keys here
}

void iSpecialKeyboard(unsigned char key)
{
    // place your codes for other keys here
}

int main()
{
    Point* p = (Point*)malloc(sizeof(Point) * 5);
    p[0]     = {10, 10};
    p[1]     = {160, 110};
    p[2]     = {10, 110};
    p[3]     = {110, 10};
    p[4]     = {60, 160};
    path     = createPath(p, 5, {255, 255, 0}, {255, 255, 255}, 2, 1.0, 1);
    printf("edge count: %d\n", path->n_edges);
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
