#include "chicken.h"
#include "basket.h"
#include "drop.h"
#include "perk.h"

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