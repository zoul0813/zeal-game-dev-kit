#include "internal.h"

const highscore_t *hiscore_get(uint8_t rank) {
    return rank < HISCORES_COUNT ? &hiscore_table[rank] : NULL;
}
