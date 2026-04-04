#include <core.h>
#include "zgdk/utils/log.h"
#include "private.h"

void tracker_log_io_error(const char* operation, uint8_t step, zos_err_t err)
{
    char step_buf[4];
    itoa(step, step_buf, 10, 'A');
    log_str("failed to ", operation);
    log_str("step ", step_buf);
    log_error("error", err);
}
