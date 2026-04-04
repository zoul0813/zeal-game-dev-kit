#include <core.h>
#include "zgdk/utils/log.h"

void log_str(const char* message, const char* value)
{
    put_s(message);
    put_s(value);
    put_s("\n");
}
