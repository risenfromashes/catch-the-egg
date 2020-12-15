#include "physics.h"
#include "svg.h"

#define FLYING_FRAMES   30
#define FLIPPING_FRAMES 15

void findMinBounds(SVGPathGroup* g, TransformMat mat, Point* min, Point* max)
{
    if (!g->hidden) {
        if (g->isGroup) {
            for (SVGPathGroup* c = (SVGPathGroup*)g->child; c; c = c->next)
                findMinBounds(c, mat, min, max);
        }
        else if (g->child) {
            Path* path = (Path*)g->child;
            for (int i = 0; i < path->n_points; i++) {
                Point p = transform(mat, path->points[i]);
                *min    = pmin(*min, p);
                *max    = pmax(*max, p);
            }
        }
    }
}

void minBounds(SVGObject* s, TransformMat mat)
{
    s->viewBox.min = {INFINITY, INFINITY};
    s->viewBox.max = neg(s->viewBox.min);
    findMinBounds(s->pathGroup, mat, &s->viewBox.min, &s->viewBox.max);
}

typedef struct _Chicken {
    SVGObject*   still;
    SVGObject*   flying[FLYING_FRAMES];
    SVGObject*   flipping[FLIPPING_FRAMES];
    Rope*        rope;
    int          rope_pos, next_pos;
    double       altitude;
    double       mass;
    int          flipped;
    int          inFlip;
    int          inFlight;
    int          wouldFly;
    double       flip_t;
    double       flip_T;
    double       flight_t;
    double       flight_T;
    Point        flight_start_p;
    Point        flight_peak;
    TransformMat localTransform, reflectTransform;
} Chicken;

Chicken* createChicken(double altitude);
Vec      chickenPosition(Chicken* chicken);
void     drawChicken(Chicken* chicken);
void     flyChicken(Chicken* chicken);
void     flipChicken(Chicken* chicken);

Chicken* createChicken(double altitude)
{
    srand((unsigned int)time(NULL));
    Chicken* chicken  = (Chicken*)malloc(sizeof(Chicken));
    chicken->altitude = altitude;
    char path[64];
    for (int i = 0; i < FLYING_FRAMES; i++) {
        sprintf(path, "assets/flying/chickenflight_%d.svg", i + 1);
        chicken->flying[i] = SVGParse(path);
    }
    for (int i = 0; i < FLIPPING_FRAMES; i++) {
        sprintf(path, "assets/flipping/chickenflip_%d.svg", i + 1);
        chicken->flipping[i] = SVGParse(path);
    }
    chicken->still    = chicken->flying[0];
    chicken->wouldFly = chicken->flipped = chicken->inFlip = chicken->inFlight = 0;
    chicken->localTransform                                                    = scaleMat({0.175, 0.175});
    minBounds(chicken->still, chicken->localTransform);
    chicken->localTransform   = matMul(translateMat(neg(chicken->still->viewBox.min)), chicken->localTransform);
    chicken->reflectTransform = matMul(translateMat({chicken->still->viewBox.max.x, 0}), scaleMat({-1, 1}));
    chicken->rope             = createRope({0, altitude}, {1280, altitude}, 2e9, 5e-5, 5.75e-8, -1, 100);
    chicken->flight_T         = 0.75;
    chicken->flip_T           = 0.1;
    chicken->mass             = 0.5;
    chicken->rope_pos         = 5 + rand() % (chicken->rope->n - 10);
    chicken->rope->ext_forces[chicken->rope_pos] =
        chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, -chicken->mass * g / 2};
    return chicken;
}
Vec chickenPosition(Chicken* chicken)
{
    Point p1    = chicken->rope->r[chicken->rope_pos];
    Point p2    = chicken->rope->r[chicken->rope_pos + 2];
    Point drope = mul(add(p1, p2), 0.5);
    int   s     = 1 - 2 * chicken->flipped;
    return {drope.x - chicken->still->viewBox.max.x / 2 + s * 1.65 * (p2.x - p1.x) / 2,
            drope.y - (chicken->still->viewBox.max.y - chicken->still->viewBox.min.y) * 0.025};
}

void drawChicken(Chicken* chicken)
{
    Vec        dr           = chickenPosition(chicken);
    double     time         = iGetTime();
    int        toggleflip   = 0;
    SVGObject* chickenFrame = chicken->still;
    if (chicken->inFlip) {
        double dt    = time - chicken->flip_t;
        int    frame = round(dt / chicken->flip_T * FLIPPING_FRAMES);
        if (dt > chicken->flip_T || frame >= FLIPPING_FRAMES) {
            chicken->inFlip = 0;
            toggleflip      = 1;
            chickenFrame    = chicken->flipping[FLIPPING_FRAMES - 1];
        }
        else
            chickenFrame = chicken->flipping[frame];
    }
    else if (chicken->inFlight) {
        double dt    = time - chicken->flight_t;
        double t     = dt / chicken->flight_T;
        double m     = 6.5;
        int    frame = (int)round(FLYING_FRAMES * (m * t + (1.5 - m) * t * t / 2)) % FLYING_FRAMES;
        Vec    fdr   = mul(chicken->flight_start_p, (1 - t) * (1 - t));
        fdr          = add(fdr, mul(chicken->flight_peak, 2 * t * (1 - t)));
        fdr          = add(fdr, mul(dr, t * t));
        if (t > 1.0 || norm(sub(fdr, dr)) < 5) {
            chicken->inFlight = 0;
            chicken->rope->ext_forces[chicken->rope_pos] =
                chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, -chicken->mass * g / 2};
        }
        else {
            dr           = fdr;
            chickenFrame = chicken->flying[frame];
        }
    }
    TransformMat mat =
        matMul(translateMat(dr),
               chicken->flipped ? matMul(chicken->reflectTransform, chicken->localTransform) : chicken->localTransform);
    iSetColor(0, 0, 0);
    iPath(chicken->rope->r, chicken->rope->n + 1, 5);
    renderSVGObject(chickenFrame, mat);
    updateWorld(chicken->rope, NULL);
    if (toggleflip) {
        chicken->flipped = !chicken->flipped;
        if (chicken->wouldFly) flyChicken(chicken);
    }
}

void flyChicken(Chicken* chicken)
{
    assert(!chicken->inFlip);
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
    chicken->inFlight                            = 1;
    chicken->flight_t                            = iGetTime();
    chicken->flight_start_p                      = chickenPosition(chicken);
    chicken->rope->ext_forces[chicken->rope_pos] = chicken->rope->ext_forces[chicken->rope_pos + 2] = {0, 0};
    chicken->rope_pos                                                                               = chicken->next_pos;
    Point mid            = mul(add(chicken->flight_start_p, chickenPosition(chicken)), 0.5);
    chicken->flight_T    = fabs(mid.x - chicken->flight_start_p.x) * 2 / 1280 * 2.25;
    chicken->flight_peak = add(mid, {0, 150});
}

void flipChicken(Chicken* chicken)
{
    assert(!chicken->inFlight);
    chicken->inFlip = 1;
    chicken->flip_t = iGetTime();
}