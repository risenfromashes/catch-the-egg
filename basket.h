#include "svg.h"

SVGObject* BasketTop;
SVGObject* BasketBottom;

typedef struct {
    double       x_pos;
    double       v_x;
    Interval     edge_l, edge_r;
    TransformMat localTransform;
    int          moving;
} Basket;

void loadBasketAssets()
{
    BasketTop    = SVGParse("assets/basket/basket_top.svg");
    BasketBottom = SVGParse("assets/basket/basket_bottom.svg");
}
Basket* createBasket()
{
    Basket* basket         = (Basket*)malloc(sizeof(Basket));
    basket->localTransform = scaleMat({0.75, 0.75});
    basket->x_pos          = 0.0;
    basket->v_x            = 5;
    basket->moving         = 0;
    SVGMinBounds(BasketTop, basket->localTransform);
    basket->localTransform =
        matMul(translateMat({-BasketTop->viewBox.min.x, 5 - BasketTop->viewBox.min.y}), basket->localTransform);
    return basket;
}

Point basketPosition(Basket* basket)
{
    if (basket->moving > 0) basket->x_pos += basket->v_x;
    if (basket->moving < 0) basket->x_pos -= basket->v_x;
    if (basket->moving) {
        if (basket->x_pos < 0) basket->x_pos = 0.0;
        if (basket->x_pos > (1280 - BasketTop->viewBox.max.x + BasketTop->viewBox.min.x))
            basket->x_pos = (1280 - BasketTop->viewBox.max.x + BasketTop->viewBox.min.x);
    }
    return {basket->x_pos, 0};
}

void drawBasketBottom(Basket* basket)
{
    TransformMat mat = matMul(translateMat(basketPosition(basket)), basket->localTransform);
    renderSVGObject(BasketBottom, mat);
}
void drawBasketTop(Basket* basket)
{
    TransformMat mat = matMul(translateMat(basketPosition(basket)), basket->localTransform);
    renderSVGObject(BasketTop, mat);
}

void controlBasket(Basket* basket, unsigned char key, int down)
{
    if (down) {
        switch (key) {
            case GLUT_KEY_RIGHT: basket->moving = 1; break;
            case GLUT_KEY_LEFT: basket->moving = -1; break;
            default: break;
        }
    }
    else
        basket->moving = 0;
}