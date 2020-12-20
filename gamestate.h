#pragma once

#include "chicken.h"
#include "basket.h"
#include "drop.h"
#include "perk.h"
#include "wind.h"

SVGObject* Background;
int        assetsLoaded = 0;

#define N_GAME_FORMAT 6

typedef enum {
    ONE_THIRTY_X1,
    ONE_THIRTY_X2,
    TWO_ZERO_X1,
    TWO_ZERO_X2,
    TWO_THIRTY_X1,
    TWO_THIRTY_X2,
} GameFormat;

typedef struct {
    GameFormat   format;
    int          n_chickens;
    Chicken*     chicken[2];
    Basket*      basket;
    Drop*        drops;
    Perk*        perks;
    int          score;
    Wind         wind;
    double       drag;
    double       t, t0;
    double       start_t;
    double       pause_t;
    int          paused;
    double       duration;
    unsigned int perkFlags;
} GameState;

void loadAssets()
{
    Background = SVGParse("assets/scene/scene.svg");
    loadWindFrames();
    loadChickenAssets();
    loadBasketAssets();
    loadDropAssets();
    assetsLoaded = 1;
}

void resumeGame(GameState* state);

GameState* createGame(GameFormat format)
{
    if (!assetsLoaded) loadAssets();
    GameState* state = (GameState*)malloc(sizeof(GameState));
    state->format    = format;
    state->pause_t = state->start_t = state->t = state->t0 = iGetTime();
    switch (format) {
        case ONE_THIRTY_X1:
            state->n_chickens = 1;
            state->duration   = 90;
            break;
        case ONE_THIRTY_X2:
            state->n_chickens = 2;
            state->duration   = 90;
            break;
        case TWO_ZERO_X1:
            state->n_chickens = 1;
            state->duration   = 120;
            break;
        case TWO_ZERO_X2:
            state->n_chickens = 2;
            state->duration   = 120;
            break;
        case TWO_THIRTY_X1:
            state->n_chickens = 1;
            state->duration   = 150;
            break;
        case TWO_THIRTY_X2:
            state->n_chickens = 2;
            state->duration   = 150;
            break;
    }
    if (state->n_chickens == 2) {
        state->chicken[0] = createChicken(450);
        state->chicken[1] = createChicken(600);
    }
    else {
        state->chicken[0] = createChicken(500);
    }
    state->wind      = {.active = 0, .T = 1.3, .F = 5};
    state->drag      = 0.0095;
    state->paused    = 0;
    state->score     = 0;
    state->basket    = createBasket();
    state->drops     = NULL;
    state->perks     = NULL;
    state->perkFlags = 0;
    return state;
}

void generateDrop(GameState* state)
{
    static int drop_his = 0;
    int        dropProb = 2 * (1 - drop_his / 500.0);
    int        windProb = 7;
    int        r        = rand() % 1000;
    if (r < dropProb) {
        drop_his += 500;
        Drop* drop = createDrop((DropType)(N_CHICKEN_DROPS + rand() % (N_DROPS - N_CHICKEN_DROPS)),
                                {30.0 + (double)(rand() % (1280 - 30)), 720.0});
        addToDropList(&state->drops, drop);
    }
    if ((r > (1000 - windProb)) && !state->wind.active)
        activateWind(&state->wind);
    else
        drop_his--;
}
Drop* disturbChicken(Chicken* chicken)
{
    static int cons_fly = 0;
    static int cons_egg = 0;
    int        r        = rand() % 1000;
    int        flyProb  = 50 * (cons_egg + 1 - 2 * cons_fly);
    int        eggProb  = 55 * (cons_fly + 1 - 2 * cons_egg);
    if (r < eggProb) {
        cons_fly = 0;
        cons_egg++;
        return layEgg(chicken);
    }
    else if (r < eggProb + flyProb) {
        cons_egg = 0;
        cons_fly++;
        flyChicken(chicken);
    }
    return NULL;
}
void handlePerkAdd(GameState* state, PerkType type)
{
    switch (type) {
        case PERK_SIZEUP:
            state->basket->scale *= 1.5;
            resizeBasket(state->basket);
            break;
        case PERK_SPEEDUP: state->basket->v_x *= 1.5; break;
        case PERK_PARACHUTE: state->drag *= 2; break;
    }
    addToPerkList(&state->perks, createPerk(type));
}
void handlePerkSub(GameState* state, PerkType type)
{
    switch (type) {
        case PERK_SIZEUP:
            state->basket->scale /= 1.5;
            resizeBasket(state->basket);
            break;
        case PERK_SPEEDUP: state->basket->v_x /= 1.5; break;
        case PERK_PARACHUTE: state->drag /= 2; break;
    }
}
void removePerks(GameState* state, double t)
{
    Perk *perk, *next = state->perks;
    while (next) {
        perk = next;
        next = perk->next;
        if (perk->start_t + perk->duration <= t) {
            handlePerkSub(state, perk->type);
            removeFromPerkList(&state->perks, perk);
        }
    }
}
void handleCatch(GameState* state, Drop* drop)
{
    switch (drop->type) {
        case DROP_GOLDEN_EGG: state->score += 10; break;
        case DROP_BLUE_EGG: state->score += 5; break;
        case DROP_NORMAL_EGG: state->score += 1; break;
        case DROP_SHIT: state->score -= 10; break;
        case DROP_CLOCK: state->duration += 5; break;
        case DROP_PARACHUTE:
        case DROP_SPEEDUP:
        case DROP_SIZEUP: handlePerkAdd(state, toPerk(drop->type)); break;
    }
    if (drop->type == DROP_SHIT)
        playSFX(SFX_SHIT);
    else
        playSFX(SFX_BASKET);
    state->score = max(state->score, 0);
}
void removeDrops(GameState* state)
{
    Drop *drop, *next = state->drops;
    int   caught = 0;
    while (next) {
        drop = next;
        next = drop->next;
        if (drop->p.y < state->basket->height) {
            double   w   = DropWidths[drop->type];
            Interval db  = {drop->p.x - w / 2, drop->p.x + w / 2};
            Interval bd  = {state->basket->edge_l.l, state->basket->edge_r.r};
            Interval ibd = {state->basket->edge_l.r, state->basket->edge_r.l};
            int      in  = 0;
            int      pIn = hasOverlap(db, bd);
            if (drop->inside == 1 && !inside(db, ibd))
                goto catchAndRemove;
            else if (drop->inside == -1 && hasOverlap(db, ibd))
                goto remove;
            else if (drop->p.y > 10) {
                if (!(in = inside(db, ibd))) {
                    if (hasOverlap(db, state->basket->edge_l)) {
                        if (in = drop->p.x >= state->basket->edge_l.l)
                            drop->p.x += (state->basket->edge_l.r - drop->p.x + w / 2 + 2);
                        else
                            drop->p.x -= (drop->p.x + w / 2 - state->basket->edge_l.l + 2);
                    }
                    else if (hasOverlap(db, state->basket->edge_r)) {
                        if (in = drop->p.x <= state->basket->edge_r.r)
                            drop->p.x -= (drop->p.x + w / 2 - state->basket->edge_r.l + 2);
                        else
                            drop->p.x += (state->basket->edge_r.r - drop->p.x + w / 2 + 2);
                    }
                }
                drop->inside = 2 * in - 1;
                continue;
            }
            else if (inside(db, ibd) || drop->inside == 1)
                goto catchAndRemove;
            else if (drop->p.y <= 0)
                goto remove;
            else
                continue;
        catchAndRemove:
            caught = 1;
            handleCatch(state, drop);
        remove:
            if (!caught) playSFX(SFX_FALL);
            removeFromDropList(&state->drops, drop);
        }
    }
}

void       saveGameState(GameState* state);
GameState* loadGameState();

void pauseGame(GameState* state)
{
    state->paused  = 1;
    state->pause_t = iGetTime();
}
void updateTimes(GameState* state, double t0, double t)
{
    state->start_t    = t - (t0 - state->start_t);
    state->t0         = t - (t0 - state->t0);
    state->wind.start = t - (t0 - state->wind.start);
    for (int i = 0; i < state->n_chickens; i++) {
        for (int j = 0; j < N_CHICKEN_ACTIVITY; j++) {
            state->chicken[i]->t[j] = t - (t0 - state->chicken[i]->t[j]);
        }
    }
}

void resumeGame(GameState* state)
{
    if (!assetsLoaded) loadAssets();
    resizeBasket(state->basket);
    state->t      = iGetTime();
    state->paused = 0;
    updateTimes(state, state->pause_t, state->t);
}

void drawFrame(GameState* state)
{
    if (!state->paused) {
        state->t  = iGetTime();
        double dt = state->t - state->t0;
        generateDrop(state);
        Drop* chickenDrop;
        for (int i = 0; i < state->n_chickens; i++)
            if (!chickenBusy(state->chicken[i]))
                if (chickenDrop = disturbChicken(state->chicken[i])) addToDropList(&state->drops, chickenDrop);
        renderSVGObject(Background, identity());
        drawBasketBottom(state->basket);
        drawBasketBottom(state->basket);
        updateDrops(state->drops, state->wind.active * state->wind.F * state->wind.dir, state->drag, dt);
        removeDrops(state);
        removePerks(state, state->t);
        drawDrops(state->drops);
        drawBasketTop(state->basket);
        for (int i = 0; i < state->n_chickens; i++)
            drawChicken(state->chicken[i], state->t, dt);
        if (state->wind.active) drawWind(&state->wind, state->t);
        state->t0 = state->t;
    }
}

void keyDown(GameState* state, unsigned char key, int special)
{
    static int saved = 0;
    if (!special && tolower(key) == 's')
        saveGameState(state);
    else if (!special && tolower(key) == 'p') {
        if (state->paused)
            resumeGame(state);
        else
            pauseGame(state);
    }
    else
        controlBasket(state->basket, key, special, 1);
}

void keyUp(GameState* state, unsigned char key) { controlBasket(state->basket, key, 0, 0); }

// structure
// state - sizeof(GameState)
// n_chicken-sizeof(int)
// n_drops - sizeof(int)
// n_perks - sizeof(int)
// chickens - sizeof(Chicken) * n_chicken
// basket - sizeof(Basket)
// drops - sizeof(drop) * n_drops
// perks - sizeof(Perk) * n_perks

void saveGameState(GameState* state)
{
    if (!state->paused) pauseGame(state);
    FILE* fp = fopen("save.dat", "wb");
    fwrite(state, sizeof(GameState), 1, fp);
    int n_chicken = state->n_chickens;
    int n_drops   = countDrops(state->drops);
    int n_perks   = countPerks(state->perks);
    fwrite(&n_chicken, sizeof(int), 1, fp);
    fwrite(&n_drops, sizeof(int), 1, fp);
    fwrite(&n_perks, sizeof(int), 1, fp);
    for (int i = 0; i < n_chicken; i++)
        fwrite(state->chicken[i], sizeof(Chicken), 1, fp);
    fwrite(state->basket, sizeof(Basket), 1, fp);
    Drop* drop = state->drops;
    while (drop) {
        fwrite(drop, sizeof(Drop), 1, fp);
        drop = drop->next;
    }
    Perk* perk = state->perks;
    while (perk) {
        fwrite(perk, sizeof(Perk), 1, fp);
        perk = perk->next;
    }
    fclose(fp);
}
GameState* loadGameState()
{
    FILE* fp = fopen("save.dat", "rb");
    if (!fp) return NULL;
    GameState* state = (GameState*)malloc(sizeof(GameState));
    fread(state, sizeof(GameState), 1, fp);
    if (feof(fp) || ferror(fp)) {
        free(state);
        return NULL;
    }
    int n_chickens, n_drops, n_perks;
    fread(&n_chickens, sizeof(int), 1, fp);
    fread(&n_drops, sizeof(int), 1, fp);
    fread(&n_perks, sizeof(int), 1, fp);
    if (feof(fp) || ferror(fp)) {
        free(state);
        return NULL;
    }
    // assert(n_chickens == state->n_chickens);
    for (int i = 0; i < n_chickens; i++) {
        state->chicken[i] = (Chicken*)malloc(sizeof(Chicken));
        fread(state->chicken[i], sizeof(Chicken), 1, fp);
        if (feof(fp) || ferror(fp)) {
            for (int j = 0; j <= i; j++) {
                if (j < i) freeRope(state->chicken[j]->rope);
                free(state->chicken[j]);
            }
            free(state);
            return NULL;
        }
        initRope(state->chicken[i]);
    }
    state->basket = (Basket*)malloc(sizeof(Basket));
    fread(state->basket, sizeof(Basket), 1, fp);
    if (feof(fp) || ferror(fp)) {
        for (int j = 0; j < n_chickens; j++) {
            freeRope(state->chicken[j]->rope);
            free(state->chicken[j]);
        }
        free(state->basket);
        free(state);
        return NULL;
    }
    state->drops = NULL;
    for (int i = 0; i < n_drops; i++) {
        Drop* drop = (Drop*)malloc(sizeof(Drop));
        fread(drop, sizeof(Drop), 1, fp);
        if (feof(fp) || ferror(fp)) {
            for (int j = 0; j < n_chickens; j++) {
                freeRope(state->chicken[j]->rope);
                free(state->chicken[j]);
            }
            free(state->basket);
            free(drop);
            freeDropList(state->drops);
            free(state);
            return NULL;
        }
        addToDropList(&state->drops, drop);
    }
    state->perks = NULL;
    for (int i = 0; i < n_perks; i++) {
        Perk* perk = (Perk*)malloc(sizeof(Perk));
        fread(perk, sizeof(Perk), 1, fp);
        if (feof(fp) || ferror(fp)) {
            for (int j = 0; j < n_chickens; j++) {
                freeRope(state->chicken[j]->rope);
                free(state->chicken[j]);
            }
            free(state->basket);
            freeDropList(state->drops);
            free(perk);
            freePerkList(state->perks);
            free(state);
            return NULL;
        }
        addToPerkList(&state->perks, perk);
    }
    fclose(fp);
    return state;
}

int isFinished(GameState* state) { return state->t >= (state->start_t + state->duration); }

void freeGameState(GameState* state)
{
    for (int j = 0; j < state->n_chickens; j++) {
        freeRope(state->chicken[j]->rope);
        free(state->chicken[j]);
    }
    free(state->basket);
    freeDropList(state->drops);
    freePerkList(state->perks);
    free(state);
}