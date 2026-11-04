#include <core.h>
#include "internal.h"

void hiscore_init(const hiscore_config_t *config) {
    hiscore_config = config;

    if(hiscore_config->default_scores != NULL) {
        mem_cpy(hiscore_table, hiscore_config->default_scores, sizeof(hiscore_table));
    } else {
        mem_set(hiscore_table, 0, sizeof(hiscore_table));
    }

    if(hiscore_config->path != NULL) {
        hiscore_load(hiscore_config->path);
    }
}
