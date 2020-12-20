#pragma once

#include "svg.h"
#include "sound.h"
#define N_WIND_FRAMES 15

SVGObject*   WindFrames[N_WIND_FRAMES];
TransformMat WindTransform;
double       WindStart = -200, WindEnd = 1480;
void         loadWindFrames()
{
    char path[64];
    for (int i = 0; i < N_WIND_FRAMES; i++) {
        sprintf(path, "assets/wind/wind_%d.svg", i + 1);
        WindFrames[i] = SVGParse(path);
    }
    SVGMinBounds(WindFrames[0], scaleMat({0.3, 0.3}));
    WindTransform = matMul(translateMat(add(neg(WindFrames[0]->viewBox.min), {0, 360})), scaleMat({0.3, 0.3}));
}

typedef struct {
    int    active;
    double T;
    double F;
    double start;
    double dir;
} Wind;

void activateWind(Wind* wind)
{
    wind->active = 1;
    wind->start  = iGetTime();
    wind->dir    = 2 * (rand() % 2) - 1;
    playSFX(SFX_WIND);
}

void drawWind(Wind* wind, double t)
{
    if ((t - wind->start) > wind->T) {
        wind->active = 0;
        return;
    }
    double x = 0;
    if (wind->dir > 0)
        x = WindStart + (t - wind->start) / wind->T * (WindEnd - WindStart);
    else
        x = WindEnd - (t - wind->start) / wind->T * (WindEnd - WindStart);
    TransformMat mat           = matMul(translateMat({x, 0}), WindTransform);
    int          frame         = (int)((t - wind->start) / wind->start * 480) % 15;
    WindFrames[frame]->opacity = 0.5;
    renderSVGObject(WindFrames[frame], mat);
}