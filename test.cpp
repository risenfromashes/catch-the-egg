#include "svg.h"
int width = 1280, height = 720;

SVGPathGroup* g;

void iDraw()
{
    double t = iGetTime();
    iClear();
    renderSVGPathGroup(g, matMul(translateMat({0, 700}), scaleMat({0.7, -0.7})));
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
    g = SVGParse("assets/test_chicken.svg");
    printf("done parsing\n");
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
