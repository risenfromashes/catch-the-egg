#pragma once

#include "chicken.h"
#include "basket.h"
#include "drop.h"
#include "perk.h"

SVGObject* Background;

typedef struct {
    int          n_chickens;
    Chicken*     chicken[3];
    Basket*      basket;
    Drop*        drops;
    Perk*        perks;
    unsigned int perkFlags;
} GameState;

void loadAssets()
{
    Background = SVGParse("assets/scene/scene.svg");
    loadChickenAssets();
    loadBasketAssets();
    loadDropAssets();
}

GameState* createGame()
{
    GameState* state  = (GameState*)malloc(sizeof(GameState));
    state->n_chickens = 1;
    state->chicken[0] = createChicken(500);
    state->basket     = createBasket();
    state->drops      = NULL;
    state->perks      = NULL;
    state->perkFlags  = 0;
    return state;
}

void generateDrop(GameState* state)
{
    static int drop_his = 0;
    int        dropProb = 2 * (1 - drop_his / 500.0);
    int        r        = rand() % 1000;
    if (r < dropProb) {
        drop_his += 500;
        Drop* drop = createDrop((DropType)(N_CHICKEN_DROPS + rand() % (N_DROPS - N_CHICKEN_DROPS)),
                                {30.0 + (double)(rand() % (1280 - 30)), 720.0});
        addToDropList(&state->drops, drop);
    }
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
void handleCatch(Drop* drop) {}
void removeDrops(GameState* state)
{
    Drop *drop, *next = state->drops;
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
            else if (inside(db, bd))
                goto catchAndRemove;
            else if (drop->p.y <= 0)
                goto remove;
        catchAndRemove:
            handleCatch(drop);
        remove:
            removeFromDropList(&state->drops, drop);
        }
    }
}

void drawFrame(GameState* state)
{
    generateDrop(state);
    Drop* chickenDrop;
    for (int i = 0; i < state->n_chickens; i++)
        if (!chickenBusy(state->chicken[i]))
            if (chickenDrop = disturbChicken(state->chicken[i])) addToDropList(&state->drops, chickenDrop);
    renderSVGObject(Background, identity());
    drawBasketBottom(state->basket);
    drawBasketBottom(state->basket);
    updateDrops(state->drops, 0.0 /*wind*/);
    removeDrops(state);
    drawDrops(state->drops);
    drawBasketTop(state->basket);
    for (int i = 0; i < state->n_chickens; i++)
        drawChicken(state->chicken[i]);
}

void keyDown(GameState* state, unsigned char key) { controlBasket(state->basket, key, 1); }

void keyUp(GameState* state, unsigned char key) { controlBasket(state->basket, key, 0); }

// file structure
// n_chicken - sizeof(int)
// n_drops - sizeof(int)
// n_perks - sizeof(int)
// chickens - sizeof(Chicken) * n_chicken
// basket - sizeof(Basket)
// drops - sizeof(drop) * n_drops
// perks - sizeof(Perk) * n_perks
// perkFlags - sizeof(unsigned int)

GameState* loadGame()
{
    FILE* fp = fopen("save.dat", "rb");
    if (!fp) return NULL;
    GameState* state = (GameState*)malloc(sizeof(GameState));
    memset(state, 0, sizeof(GameState));
    int n_chickens, n_drops, n_perks;
    fread(&n_chickens, sizeof(int), 1, fp);
    fread(&n_drops, sizeof(int), 1, fp);
    fread(&n_perks, sizeof(int), 1, fp);
    if (feof(fp) || ferror(fp)) goto failure;
    state->n_chickens = n_chickens;
    for (int i = 0; i < n_chickens; i++) {
        state->chicken[i] = (Chicken*)malloc(sizeof(Chicken));
        fread(state->chicken[i], sizeof(Chicken), 1, fp);
        if (feof(fp) || ferror(fp)) goto failure;
        initRope(state->chicken[i]);
    }
    state->basket = (Basket*)malloc(sizeof(Basket));
    fread(state->basket, sizeof(Basket), 1, fp);
    if (feof(fp) || ferror(fp)) goto failure;
    resizeBasket(state->basket);
    for (int i = 0; i < n_drops; i++) {
        Drop* drop = (Drop*)malloc(sizeof(Drop));
        fread(drop, sizeof(Drop), 1, fp);
        if (feof(fp) || ferror(fp)) goto failure;
        addToDropList(&state->drops, drop);
    }
    for (int i = 0; i < n_perks; i++) {
        Perk* perk = (Perk*)malloc(sizeof(Perk));
        fread(perk, sizeof(Perk), 1, fp);
        if (feof(fp) || ferror(fp)) goto failure;
        addToPerkList(&state->perks, perk);
    }
    fread(&state->perkFlags, sizeof(unsigned int), 1, fp);
    if (feof(fp) || ferror(fp)) goto failure;
    goto success;
failure:
    for (int i = 0; i < n_chickens; i++)
        if (state->chicken[i]) free(state->chicken[i]);
    if (state->basket) free(state->basket);
    freeDropList(state->drops);
    freePerkList(state->perks);
    free(state);
    return NULL;
success:
    return state;
}
