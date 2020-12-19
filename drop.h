#pragma once

#include "svg.h"
#include "physics.h"

#define N_CHICKEN_DROPS 4
#define N_DROPS         8

typedef enum {
    DROP_GOLDEN_EGG,
    DROP_BLUE_EGG,
    DROP_NORMAL_EGG,
    DROP_SHIT,
    DROP_PARACHUTE,
    DROP_SPEEDUP,
    DROP_SIZEUP,
    DROP_CLOCK
} DropType;

SVGObject* DropObjects[N_DROPS];
double     DropWidths[N_DROPS];
double     DropHeights[N_DROPS];

void loadDropAssets()
{
    TransformMat scales[N_DROPS];
    for (int i = 0; i < N_DROPS; i++)
        scales[i] = scaleMat({0.22, 0.22});
    DropObjects[DROP_GOLDEN_EGG] = SVGParse("assets/drops/golden_egg.svg");
    DropObjects[DROP_BLUE_EGG]   = SVGParse("assets/drops/blue_egg.svg");
    DropObjects[DROP_NORMAL_EGG] = SVGParse("assets/drops/normal_egg.svg");
    DropObjects[DROP_SHIT]       = SVGParse("assets/drops/chicken_shit.svg");
    DropObjects[DROP_PARACHUTE]  = SVGParse("assets/drops/parachute.svg");
    DropObjects[DROP_SPEEDUP]    = SVGParse("assets/drops/speedup.svg");
    DropObjects[DROP_SIZEUP]     = SVGParse("assets/drops/sizeup.svg");
    DropObjects[DROP_CLOCK]      = SVGParse("assets/drops/clock.svg");
    scales[DROP_SIZEUP]          = scaleMat({0.4, 0.4});
    scales[DROP_CLOCK]           = scaleMat({0.35, 0.35});
    scales[DROP_SPEEDUP]         = scaleMat({0.3, 0.3});
    scales[DROP_PARACHUTE]       = scaleMat({0.35, 0.35});
    for (int i = 0; i < N_DROPS; i++) {
        SVGMinBounds(DropObjects[i], scales[i]);
        DropWidths[i]  = DropObjects[i]->viewBox.max.x - DropObjects[i]->viewBox.min.x;
        DropHeights[i] = DropObjects[i]->viewBox.max.y - DropObjects[i]->viewBox.min.y;
        DropObjects[i]->localTransform =
            matMul(translateMat({-DropWidths[i] / 2 - DropObjects[i]->viewBox.min.x, -DropObjects[i]->viewBox.min.y}),
                   scales[i]);
    }
}

typedef struct _Drop {
    double        t0;
    DropType      type;
    Vec           p, v;
    int           inside;
    struct _Drop *next, *prev;
} Drop;

Drop* createDrop(DropType type, Vec position)
{
    Drop* drop = (Drop*)malloc(sizeof(Drop));
    drop->type = type;
    drop->p    = position;
    drop->v    = {0, 0};
    drop->prev = drop->next = NULL;
    drop->inside            = 0;
    return drop;
}

int countDrops(Drop* head)
{
    int   i    = 0;
    Drop* drop = head;
    while (drop)
        drop = drop->next, i++;
    return i;
}

void addToDropList(Drop** head, Drop* drop)
{
    if (*head) {
        drop->prev       = NULL;
        drop->next       = *head;
        drop->next->prev = drop;
    }
    else
        drop->prev = drop->next = NULL;
    *head = drop;
}
void removeFromDropList(Drop** head, Drop* drop)
{
    assert(*head);
    if (drop->next) drop->next->prev = drop->prev;
    if (drop->prev) drop->prev->next = drop->next;
    if (*head == drop) *head = drop->next;
    free(drop);
}

void updateDrops(Drop* head, double wind, double dt)
{
    dt *= 10;
    Drop* drop = head;
    while (drop) {
        drop->p     = add(drop->p, mul(drop->v, dt));
        Vec    a    = {wind, -g};
        double drag = 0.0095;
        a           = add(a, mul(drop->v, -norm(drop->v) * drag));
        drop->v     = add(drop->v, mul(a, dt));
        drop        = drop->next;
    }
}

void drawDrops(Drop* head)
{
    Drop* drop = head;
    while (drop) {
        TransformMat mat = matMul(translateMat(drop->p), DropObjects[drop->type]->localTransform);
        renderSVGObject(DropObjects[drop->type], mat);
        // iSetColor(255, 0, 0);
        // double w = DropWidths[drop->type];
        // double h = DropHeights[drop->type];
        // iRectangle(drop->p.x - w / 2, drop->p.y, w, h);
        assert(drop->p.y > 0);
        drop = drop->next;
    }
}

void freeDropList(Drop* head)
{
    Drop *x = head, *y;
    while (x) {
        y = x;
        x = x->next;
        free(y);
    }
}