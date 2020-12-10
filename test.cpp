#include "vecdraw.h"
int width = 1280, height = 720;

Path* path;
void  iDraw()
{
    iClear();
    drawPath(path);
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
    Point* p = (Point*)malloc(sizeof(Point) * 4);
    p[0]     = {50, 50};
    p[1]     = {150, 50};
    p[2]     = {50, 150};
    p[3]     = {150, 150};
    path     = createPath(p, 4, {255, 0, 0}, {255, 255, 255}, 2, 1.0, 1);
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
