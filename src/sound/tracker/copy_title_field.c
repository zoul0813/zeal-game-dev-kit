#include <core.h>
#include "private.h"

void tracker_copy_title_field(char* dst, const char* src)
{
    mem_set(dst, CH_SPACE, TRACKER_TITLE_LEN);
    for (uint8_t i = 0; i < TRACKER_TITLE_LEN; i++) {
        if (src[i] == CH_NULL)
            break;
        dst[i] = src[i];
    }
    dst[TRACKER_TITLE_LEN] = CH_NULL;
}
