#include "svg.h"
#include "chicken.h"
#include "basket.h"

int width = 1280, height = 720;

SVGObject* scene;
Chicken*   chicken;
Rope*      rope;
Basket*    basket;

void iDraw()
{
    // static int k = 0;
    // double     t = iGetTime();
    // if ((int)(t / 0.01) > k) k++;
    iClear();
    renderSVGObject(scene, identity());
    drawChicken(chicken);
    drawBasketBottom(basket);
    drawBasketTop(basket);
}
void iMouseMove(int mx, int my) {}

void iMouse(int button, int state, int mx, int my) {}
void iPassiveMouseMove(int, int) {}
void iResize(int w, int h) {}
void iKeyboard(unsigned char key)
{
    flyChicken(chicken);
    // static int flag      = 0;
    // rope->ext_forces[22] = rope->ext_forces[24] = {0, -0.1 * g * flag};
    // flag                                        = !flag;
    // place your codes for other keys here
}

void iSpecialKeyboard(unsigned char key)
{
    controlBasket(basket, key, 1);
    // place your codes for other keys here
}

void iKeyboardUp(unsigned char key) {}
void iSpecialKeyboardUp(unsigned char key) { controlBasket(basket, key, 0); }

int main()
{
    printf("init\n");
    scene   = SVGParse("assets/scene/scene.svg");
    chicken = createChicken(500);
    basket  = createBasket();
    iSetTransparency(1);
    iInitializeEx(width, height, 0, "Demo!");
    return 0;
}
