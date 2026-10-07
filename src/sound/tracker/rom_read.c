#include <core.h>
#include "private.h"

uint8_t tracker_rom_read(tracker_rom_cursor_t *cursor, void *out, uint16_t size)
{
    if (size > cursor->remaining) return 0;
    if (out != NULL) mem_cpy(out, cursor->pos, size);
    cursor->pos += size;
    cursor->remaining -= size;
    return 1;
}
