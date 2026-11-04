#include "internal.h"

const highscore_t *hiscore_find(const char initials[3]) {
    const highscore_t *best = NULL;
    uint8_t i;
    if (!initials) return NULL;
    for (i = 0; i < HISCORES_COUNT; ++i) {
        const highscore_t *entry = &hiscore_table[i];
        if (entry->initials[0] == initials[0]
            && entry->initials[1] == initials[1]
            && entry->initials[2] == initials[2]
            && (!best || entry->score > best->score))
            best = entry;
    }
    return best;
}
