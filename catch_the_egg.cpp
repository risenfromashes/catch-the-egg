#include "physics.h"

int width = 1280, height = 720;

// Vec* Rope;
// int    n_Points    = 20;
// double Rope_width = 1000;

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
