// // Point dx = add(mul(c->v[i], dt), mul(c->a[i], 0.5 * dt * dt));
// // // if the next time stap will violate the constraints, find the last Point in time when it doesn't doesn't
// // double lim;
// // if (norm(sub(r1, add(r, dx))) > c->dp * maxst)
// //     lim = c->dp * maxst;
// // else if (norm(sub(r1, add(r, dx))) < c->dp * minst)
// //     lim = c->dp * minst;
// // else {
// //     c->constr[i - 1].r = 0;
// //     goto second;
// // }
// // {
// //     c->constr[i - 1].r = 1;
// //     Point  sol         = solveQuadratic(norm2(dx), -dot(dx, sub(r1, r)), norm2(sub(r1, r)) - lim * lim);
// //     double t;
// //     if (sol.x <= 0 && sol.y <= 0)
// //         goto second;
// //     else if (sol.x > 0 && sol.y > 0)
// //         t = max(sol.x, sol.y);
// //     if (t < dt) {
// //         dt   = t;
// //         minP = i;
// //     }
// // }
// // second : dx = add(mul(c->v[i], dt), mul(c->a[i], 0.5 * dt * dt));
// // if (norm(sub(r2, add(r, dx))) > c->dp * maxst)
// //     lim = c->dp * maxst;
// // else if (norm(sub(r2, add(r, dx))) < c->dp * minst)
// //     lim = c->dp * minst;
// // else {
// //     c->constr[i + 1].l = 0;
// //     continue;
// // }
// // {
// //     c->constr[i + 1].l = 1;
// //     Point  sol         = solveQuadratic(norm2(dx), -dot(dx, sub(r2, r)), norm2(sub(r2, r)) - lim * lim);
// //     double t;
// //     if (sol.x <= 0 && sol.y <= 0)
// //         continue;
// //     else if (sol.x > 0 && sol.y > 0)
// //         t = max(sol.x, sol.y);
// //     if (t < dt) {
// //         dt   = t;
// //         minP = i;
// //     }
// // }
// // RopeCollisionResult checkCollsion(Collider* cl, Rope* rp, int j)
// // {
// //     RopeCollisionResult ret;
// //     Point               p = rp->r[j];
// //     ret.isCollision       = 0;
// //     if (!insideAABB(cl->aabb, p)) return ret;
// //     int    l = 0, r = cl->n - 1, k;
// //     double ll = cl->angles[l], rr = cl->angles[r];
// //     double ang = angle(sub(p, cl->c));
// //     double s   = 2 * ((cl->angles[1] > cl->angles[0]) ^ (fabs(cl->angles[1] - cl->angles[0]) > PI)) - 1;
// //     if (inAngleRange(ang, cl->angles[l], cl->angles[r], s)) {
// //         do {
// //             int m = (l + r) / 2;
// //             if (inAngleRange(ang, cl->angles[l], cl->angles[m], s))
// //                 r = m;
// //             else
// //                 l = m;
// //         } while (l < r - 1);
// //         k = s > 0 ? l : r;
// //     }
// //     else
// //         k = r;
// //     Point  p1 = cl->r[k], p2 = cl->r[(k + 1) % cl->n];
// //     double d1 = signedArea(p1, p2, p), d2 = signedArea(p1, p2, cl->c);
// //     if (ret.isCollision = d1 * d2 >= 0) {
// //         if (fabs(d2 - d1) < 1e-8)
// //             ret.dr = sub(p, cl->c);
// //         else
// //             ret.dr = mul(sub(p, cl->c), d1 / (d2 - d1));
// //     }

// //     return ret;
// // }

// typedef struct _CSSGradientStop {
//     unsigned int             offset;
//     CSSColor                 color;
//     struct _CSSGradientStop* next;
// } CSSGradientStop;

// typedef struct {
//     Point p1, p2;
// } CSSLinearGradient;

// typedef struct {
//     Point c;
//     float r;
// } CSSRadialGradient;
