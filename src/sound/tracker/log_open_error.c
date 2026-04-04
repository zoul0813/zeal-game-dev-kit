#include "zgdk/utils/log.h"
#include "private.h"

void tracker_log_open_error(const char* message, const char* filename, zos_err_t err)
{
    log_str(message, filename);
    log_error("error", err);
}
