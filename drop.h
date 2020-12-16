#include "svg.h"

#define N_DROPS 8

typedef enum {
    DROP_GOLDEN_EGG,
    DROP_BLUE_EGG,
    DROP_NORMAL_EGG,
    DROP_SHIT,
    DROP_APPLE,
    DROP_SPEEDUP,
    DROP_SIZEUP,
    DROP_CLOCK
} DropType;

SVGObject* DropObjects[N_DROPS];

void loadDropAssets()
{
    DropObjects[DROP_GOLDEN_EGG] = SVGParse("assets/drops/golden_egg.svg");
    DropObjects[DROP_BLUE_EGG]   = SVGParse("assets/drops/blue_egg.svg");
    DropObjects[DROP_NORMAL_EGG] = SVGParse("assets/drops/normal_egg.svg");
    DropObjects[DROP_SHIT]       = SVGParse("assets/drops/chicken_shit.svg");
    DropObjects[DROP_APPLE]      = SVGParse("assets/drops/apple.svg");
    DropObjects[DROP_SPEEDUP]    = SVGParse("assets/drops/speedup.svg");
    DropObjects[DROP_SIZEUP]     = SVGParse("assets/drops/sizeup.svg");
    DropObjects[DROP_CLOCK]      = SVGParse("assets/drops/clock.svg");
}

typedef struct _Drop {
    DropType      type;
    Vec           position;
    struct _Drop *next, *prev;
} Drop;

Drop* createDrop(DropType type, Vec position) {}

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

void drawDrops(Drop* head) {}

void freeDropList(Drop* head) {}