#include <core.h>
#include "zgdk/utils/log.h"

void log_error(const char* message, uint8_t err)
{
    put_s(message);
    put_s(" ");
    put_u8(err);
    put_s(" (0x");
    put_hex8(err);
    put_s(")\n");
}
