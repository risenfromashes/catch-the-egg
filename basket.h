#include "svg.h"

SVGObject* BasketTop;
SVGObject* BasketBottom;

typedef struct {
    double       x_pos;
    double       v_x;
    Interval     edge_l, edge_r;
    double       width, height;
    double       scale;
    int          moving;
    TransformMat transform;
} Basket;

void resizeBasket(Basket* basket)
{
    basket->transform = scaleMat({0.75 * basket->scale, 0.75 * basket->scale});
    SVGMinBounds(BasketTop, basket->transform);
    basket->transform =
        matMul(translateMat({-BasketTop->viewBox.min.x, 5 - BasketTop->viewBox.min.y}), basket->transform);
    basket->width  = BasketTop->viewBox.max.x - BasketTop->viewBox.min.x;
    basket->height = BasketTop->viewBox.max.y - 8;
}

void loadBasketAssets()
{
    BasketTop    = SVGParse("assets/basket/basket_top.svg");
    BasketBottom = SVGParse("assets/basket/basket_bottom.svg");
}
Basket* createBasket()
{
    Basket* basket = (Basket*)malloc(sizeof(Basket));
    basket->x_pos  = 0.0;
    basket->v_x    = 5;
    basket->moving = 0;
    basket->scale  = 1.0;
    resizeBasket(basket);
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
    basket->edge_l = {basket->x_pos + 0.05 * basket->width, basket->x_pos + basket->width * 0.2};
    basket->edge_r = {basket->x_pos + 0.83 * basket->width, basket->x_pos + basket->width * 0.98};
    return {basket->x_pos, 0};
}

void drawBasketBottom(Basket* basket)
{
    TransformMat mat = matMul(translateMat(basketPosition(basket)), basket->transform);
    renderSVGObject(BasketBottom, mat);
}
void drawBasketTop(Basket* basket)
{
    TransformMat mat = matMul(translateMat(basketPosition(basket)), basket->transform);
    renderSVGObject(BasketTop, mat);
    // iSetColor(255, 0, 0);
    // iCircle(basket->edge_l.l, basket->height, 2);
    // iCircle(basket->edge_l.r, basket->height, 2);
    // iCircle(basket->edge_r.l, basket->height, 2);
    // iCircle(basket->edge_r.r, basket->height, 2);
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