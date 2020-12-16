

typedef enum { PERK_SIZEUP, PERK_GRAVITY, PERK_SPEEDUP } PerkType;

typedef struct _Perk {
    double        start_t;
    double        end_t;
    struct _Perk *prev, *next;
} Perk;

int countPerks(Perk* head)
{
    int   i    = 0;
    Perk* perk = head;
    while (perk)
        perk = perk->next, i++;
    return i;
}

void addToPerkList(Perk** head, Perk* perk)
{
    if (*head) {
        perk->prev       = NULL;
        perk->next       = *head;
        perk->next->prev = perk;
    }
    else
        perk->prev = perk->next = NULL;
    *head = perk;
}

void removeFromPerkList(Perk** head, Perk* perk)
{
    assert(*head);
    if (perk->next) perk->next->prev = perk->prev;
    if (perk->prev) perk->prev->next = perk->next;
    if (*head == perk) *head = perk->next;
    free(perk);
}

void freePerkList(Perk* head) {}