#include "svg.h"
int width = 1280, height = 720;

SVGPathGroup* g;

void iDraw()
{
    double t = iGetTime();
    iClear();
    renderSVGPathGroup(g, matMul(translateMat({400, 600}), scaleMat({0.5, -0.5})));
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
    printf("parsing began\n");
    g = SVGParse("assets/Chicken_1.svg");
    printf("done parsing\n");
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
