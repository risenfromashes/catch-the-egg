#include "svg.h"
int width = 1280, height = 720;

SVGPathGroup* scene;
SVGPathGroup* g[30];

void iDraw()
{
    static int k = 0;
    double     t = iGetTime();
    if ((int)(t / 0.01) > k) k++;
    iClear();
    renderSVGPathGroup(scene, matMul(translateMat({0, 720}), scaleMat({1, -1})));
    renderSVGPathGroup(g[k % 30], matMul(translateMat({200, 500 + 100 * sin(2 * t)}), scaleMat({0.2, -0.2})));
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
    scene = SVGParse("assets/scene/scene.svg");
    char path[64];
    for (int i = 1; i <= 30; i++) {
        sprintf(path, "assets/flying/chickenflight_%d.svg", i);
        g[i - 1] = SVGParse(path);
    }
    printf("done parsing\n");
    iSetTransparency(1);
    iInitializeEx(width, height, "Demo!");
    return 0;
}
