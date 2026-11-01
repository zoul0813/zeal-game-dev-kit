#pragma once

#include "zgdk/input/controller.h"
#include "zgdk/deprecation.h"

/**
 * @deprecated Use controller_read_pad(SNES_PORT1).
 * This reads only port 1; it does not restore the old global snapshots or
 * implicit port-2 polling. This alias is not a legacy compatibility layer.
 */
#define controller_read() \
    ZGDK_DEPRECATED_WARNING_("controller_read() is deprecated; use controller_read_pad(SNES_PORT1)") \
    controller_read_pad(SNES_PORT1)
