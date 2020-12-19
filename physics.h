#pragma once

#include "vecdraw.h"

double g = 9.8 * 2;
typedef struct {
    double k_s, k_b, k_d, dm, dp, maxp, minp;
    Vec *  r, *r0, *rc, *a;
    Vec*   ext_forces;
    char*  colliding;
    int    n;
} Rope;

void satisfyConstraints(Rope* c, int i)
{
    Vec    dr  = sub(c->r[i + 1], c->r[i]);
    double dr_ = norm(dr);
    if (c->minp <= dr_ && dr_ <= c->maxp) return;
    double delta;
    delta     = dr_ - (dr_ < c->minp ? c->minp : c->maxp);
    int first = i == 0, last = i == c->n - 1;
    c->r[i]     = add(c->r[i], mul(dr, delta / dr_ * (0.5 - 0.5 * first + 0.5 * last)));
    c->r[i + 1] = sub(c->r[i + 1], mul(dr, delta / dr_ * (0.5 + 0.5 * first - 0.5 * last)));
}

Rope* createRope(Vec p1, Vec p2, double Y, double A, double u, double length, int n_Points)
{
    Rope* c = (Rope*)malloc(sizeof(Rope));
    if (length < 0) length = norm(sub(p1, p2));
    c->dm = u, c->n = n_Points + 1, c->dp = length / c->n, c->k_s = 1;
    c->r          = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->r0         = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->rc         = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->a          = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->ext_forces = (Vec*)malloc(sizeof(Vec) * (c->n + 1));
    c->colliding  = (char*)malloc(c->n + 1);
    Vec del       = mul(sub(p2, p1), 1.0 / c->n);
    c->r0[0] = c->r[0] = p1;
    for (int i = 1; i < c->n; i++)
        c->r0[i] = c->r[i] = add(c->r[i - 1], del);
    c->r0[c->n] = c->r[c->n] = p2;
    for (int i = 0; i <= c->n; i++)
        c->a[i] = c->ext_forces[i] = {0, 0};
    return c;
}
void calcRope(Rope* c, double dt)
{
    // mass-spring model
    // verlet integration with constraints
    static int CONSTR_ITR = 4;
    c->dm                 = 0.002;
    c->k_s                = 2500;
    c->k_b                = 10000;
    c->k_d                = 0.0001;
    double minst          = 0.9999;
    double maxst          = 1.0001;
    c->minp               = minst * c->dp;
    c->maxp               = maxst * c->dp;
    static int f1 = 1, f2 = 1;
    for (int i = 1; i < c->n; i++) {
        Vec temp = c->r[i];
        c->r[i]  = add(sub(mul(c->r[i], 2), c->r0[i]), mul(c->a[i], dt * dt));
        c->r0[i] = temp;
    }
    double U = 0.0, U_ = 0.0;
    // satisfy constraints
    for (int k = 0; k < CONSTR_ITR; k++) {
        for (int i = 0; i < c->n; i++) {
            satisfyConstraints(c, i);
            // record internal energy at collision less state
        }
    }
    // collision resolution
    // resolveCollision(box, c, 0.95);

    for (int i = 1; i < c->n; i++) {
        Vec    r = c->r[i], r1 = c->r[i - 1], r2 = c->r[i + 1];
        Vec    v   = add(mul(sub(c->r[i], c->r0[i]), 1 / dt), mul(c->a[i], dt / 2));
        Vec    dr1 = sub(r1, r), dr2 = sub(r2, r);
        double dr1_ = norm(dr1), dr2_ = norm(dr2);
        /// stretch strain
        Vec F_s = add(mul(dr1, c->k_s * (1 - c->dp / dr1_)), mul(dr2, c->k_s * (1 - c->dp / dr2_)));
        //  bend strain
        double cos_ = dot(neg(dr1), dr2) / (dr1_ * dr2_);
        cos_        = cos_ < -1 ? -1 : (cos_ > 1 ? 1 : cos_);
        Vec F_b     = mul(add(mul(sub(mul(r, 2), add(r1, r2)), dr1_ * dr2_),
                          mul(add(mul(dr1, dr2_ / dr1_), mul(dr2, dr1_ / dr2_)), dot(dr1, dr2))),
                      -c->k_b / (dr1_ * dr1_ * dr2_ * dr2_) * acos(cos_));
        //  gravity
        Vec F_g = {0, -g * c->dm};
        // drag
        Vec F_d = mul(v, -norm(v) * c->k_d);
        Vec a   = mul(add(add(F_s, add(F_b, add(F_g, F_d))), c->ext_forces[i]), 1 / c->dm);
        c->a[i] = a;
    }
}

void updateRope(Rope* rp, double* t0)
{
    double t  = iGetTime();
    double Dt = (t - *t0) * 10;
    *t0       = t;
    double dt = 0.002;
    double N  = Dt / dt;
    for (int i = 0; i < N; i++) {
        calcRope(rp, dt);
    }
}
