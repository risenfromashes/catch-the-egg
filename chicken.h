#pragma once

#include "physics.h"
#include "svg.h"
#include "drop.h"

#define FLYING_FRAMES   30
#define FLIPPING_FRAMES 15
#define LAYING_FRAMES   15

SVGObject*   ChickenFlight[FLYING_FRAMES];
SVGObject*   ChickenFlip[FLIPPING_FRAMES];
SVGObject*   ChickenLaying[LAYING_FRAMES];
SVGObject*   ChickenStill;
TransformMat ChickenLocalMat;
TransformMat ChickenReflectMat;

#define N_CHICKEN_ACTIVITY 3
enum ChickenActivity { CHICKEN_FLIGHT, CHICKEN_FLIP, CHICKEN_LAYING };

typedef struct _Chicken {
    double t0;
    Rope*  rope;
    int    rope_pos, next_pos;
    double altitude;
    double mass;
    int    flipped;
    int    wouldFly;
    Point  flight_start_p;
    Point  flight_peak;
    double t[N_CHICKEN_ACTIVITY];
    double T[N_CHICKEN_ACTIVITY];
    int    in[N_CHICKEN_ACTIVITY];
} Chicken;

Chicken* createChicken(double altitude);
Vec      chickenPosition(Chicken* chicken);
void     drawChicken(Chicken* chicken);
void     flyChicken(Chicken* chicken);
void     flipChicken(Chicken* chicken);

void loadChickenAssets()
{
    char path[64];
    for (int i = 0; i < FLYING_FRAMES; i++) {
        sprintf(path, "assets/flying/chickenflight_%d.svg", i + 1);
        ChickenFlight[i] = SVGParse(path);
    }
    for (int i = 0; i < FLIPPING_FRAMES; i++) {
        sprintf(path, "assets/flipping/chickenflip_%d.svg", i + 1);
        ChickenFlip[i] = SVGParse(path);
    }
    for (int i = 0; i < LAYING_FRAMES; i++) {
        sprintf(path, "assets/laying/chickenlaying_%d.svg", i + 1);
        ChickenLaying[i] = SVGParse(path);
    }
    ChickenStill    = ChickenFlight[0];
    ChickenLocalMat = scaleMat({0.15, 0.15});
    SVGMinBounds(ChickenStill, ChickenLocalMat);
    ChickenLocalMat   = matMul(translateMat(neg(ChickenStill->viewBox.min)), ChickenLocalMat);
    ChickenReflectMat = matMul(translateMat({ChickenStill->viewBox.max.x, 0}), scaleMat({-1, 1}));
}

void initRope(Chicken* chicken)
{
    chicken->rope     = createRope({0, chicken->altitude}, {1280, chicken->altitude}, 2e9, 5e-5, 5.75e-8, -1, 100);
    chicken->rope_pos = 5 + rand() % (chicken->rope->n - 10);
    chicken->rope->ext_forces[chicken->rope_pos] =
        chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, -chicken->mass * g / 2};
    for (int i = 0; i < 1000; i++)
        updateRope(chicken->rope, &chicken->t0);
}

Chicken* createChicken(double altitude)
{
    srand((unsigned int)time(NULL));
    Chicken* chicken  = (Chicken*)malloc(sizeof(Chicken));
    chicken->wouldFly = chicken->flipped = 0;
    chicken->altitude                    = altitude;
    chicken->t0                          = 0;
    chicken->mass                        = 0.5;
    chicken->T[CHICKEN_FLIGHT]           = 0.75;
    chicken->T[CHICKEN_FLIP]             = 0.1;
    chicken->T[CHICKEN_LAYING]           = 0.35;
    for (int i = 0; i < N_CHICKEN_ACTIVITY; i++) {
        chicken->in[i] = 0;
        chicken->t[i]  = 0;
    }
    initRope(chicken);
    return chicken;
}
Vec chickenPosition(Chicken* chicken)
{
    Point p1    = chicken->rope->r[chicken->rope_pos];
    Point p2    = chicken->rope->r[chicken->rope_pos + 2];
    Point drope = mul(add(p1, p2), 0.5);
    int   s     = 1 - 2 * chicken->flipped;
    return {drope.x - ChickenStill->viewBox.max.x / 2 + s * 1.65 * (p2.x - p1.x) / 2,
            drope.y - (ChickenStill->viewBox.max.y - ChickenStill->viewBox.min.y) * 0.025};
}

Vec eggPosition(Chicken* chicken)
{
    Point p1    = chicken->rope->r[chicken->rope_pos];
    Point p2    = chicken->rope->r[chicken->rope_pos + 2];
    Point drope = mul(add(p1, p2), 0.5);
    int   s     = 1 - 2 * chicken->flipped;
    return {drope.x + s * (ChickenStill->viewBox.max.x - ChickenStill->viewBox.min.x) * 0.22,
            drope.y + (ChickenStill->viewBox.max.y - ChickenStill->viewBox.min.y) * 0.2};
}

void drawChicken(Chicken* chicken)
{
    Vec        dr           = chickenPosition(chicken);
    double     time         = iGetTime();
    int        toggleflip   = 0;
    SVGObject* chickenFrame = ChickenStill;
    for (int i = 0; i < N_CHICKEN_ACTIVITY; i++) {
        if (chicken->in[i]) {
            double dt = time - chicken->t[i];
            int    frame;
            switch ((ChickenActivity)i) {
                case CHICKEN_LAYING:
                    frame = round(dt / chicken->T[i] * LAYING_FRAMES);
                    if (dt > chicken->T[i] || frame >= LAYING_FRAMES) {
                        chicken->in[i] = 0;
                        chickenFrame   = ChickenLaying[LAYING_FRAMES - 1];
                    }
                    else
                        chickenFrame = ChickenLaying[frame];
                    break;
                case CHICKEN_FLIP:
                    frame = round(dt / chicken->T[i] * FLIPPING_FRAMES);
                    if (dt > chicken->T[i] || frame >= FLIPPING_FRAMES) {
                        chicken->in[i] = 0;
                        toggleflip     = 1;
                        chickenFrame   = ChickenFlip[FLIPPING_FRAMES - 1];
                    }
                    else
                        chickenFrame = ChickenFlip[frame];
                    break;
                case CHICKEN_FLIGHT: {
                    double t = dt / chicken->T[i];
                    double m = 6.5;
                    frame    = (int)round(FLYING_FRAMES * (m * t + (1.5 - m) * t * t / 2)) % FLYING_FRAMES;
                    Vec fdr  = mul(chicken->flight_start_p, (1 - t) * (1 - t));
                    fdr      = add(fdr, mul(chicken->flight_peak, 2 * t * (1 - t)));
                    fdr      = add(fdr, mul(dr, t * t));
                    if (t > 1.0 || norm(sub(fdr, dr)) < 5) {
                        chicken->in[i] = 0;
                        chicken->rope->ext_forces[chicken->rope_pos] =
                            chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, -chicken->mass * g / 2};
                    }
                    else {
                        dr           = fdr;
                        chickenFrame = ChickenFlight[frame];
                    }
                } break;
            }
            break;
        }
    }
    TransformMat mat =
        matMul(translateMat(dr), chicken->flipped ? matMul(ChickenReflectMat, ChickenLocalMat) : ChickenLocalMat);
    iSetColor(0, 0, 0);
    iPath(chicken->rope->r, chicken->rope->n + 1, 5);
    renderSVGObject(chickenFrame, mat);
    updateRope(chicken->rope, &chicken->t0);
    if (toggleflip) {
        chicken->flipped = !chicken->flipped;
        if (chicken->wouldFly) flyChicken(chicken);
    }
}

void flyChicken(Chicken* chicken)
{
    assert(!chicken->in[CHICKEN_FLIP]);
    if (chicken->wouldFly)
        chicken->wouldFly = 0;
    else {
        chicken->next_pos = 5 + rand() % (chicken->rope->n - 10);
        int d;
        if (abs(d = chicken->next_pos - chicken->rope_pos) < 30) {
            chicken->next_pos = (chicken->next_pos + sgn(d) * 20) % chicken->rope->n;
            if (chicken->next_pos < 5) chicken->next_pos = chicken->rope->n - 10;
            if (chicken->next_pos > chicken->rope->n - 10) chicken->next_pos = 5;
        }
        int s = 1 - 2 * chicken->flipped;
        if (chicken->next_pos * s > chicken->rope_pos * s) {
            chicken->wouldFly = 1;
            flipChicken(chicken);
            return;
        }
    }
    chicken->in[CHICKEN_FLIGHT]                  = 1;
    chicken->t[CHICKEN_FLIGHT]                   = iGetTime();
    chicken->flight_start_p                      = chickenPosition(chicken);
    chicken->rope->ext_forces[chicken->rope_pos] = chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, 0};
    chicken->rope_pos                                                                               = chicken->next_pos;
    Point mid                  = mul(add(chicken->flight_start_p, chickenPosition(chicken)), 0.5);
    chicken->T[CHICKEN_FLIGHT] = fabs(mid.x - chicken->flight_start_p.x) * 2 / 1280 * 2.25;
    chicken->flight_peak       = add(mid, {0, 150});
}

void flipChicken(Chicken* chicken)
{
    assert(!chicken->in[CHICKEN_FLIP]);
    chicken->in[CHICKEN_FLIP] = 1;
    chicken->t[CHICKEN_FLIP]  = iGetTime();
}
int chickenBusy(Chicken* chicken)
{
    for (int i = 0; i < N_CHICKEN_ACTIVITY; i++)
        if (chicken->in[i]) return 1;
    return 0;
}

Drop* layEgg(Chicken* chicken)
{
    assert(!chickenBusy(chicken));
    int      r = rand() % 100;
    DropType type;
    if (r < 7)
        type = DROP_GOLDEN_EGG;
    else if (r < 20)
        type = DROP_BLUE_EGG;
    else if (r < 80)
        type = DROP_NORMAL_EGG;
    else
        type = DROP_SHIT;
    chicken->in[CHICKEN_LAYING] = 1;
    chicken->t[CHICKEN_LAYING]  = iGetTime();
    return createDrop(type, eggPosition(chicken));
}