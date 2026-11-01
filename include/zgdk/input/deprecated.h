#pragma once

#include "zgdk/input.h"
#include "zgdk/deprecation.h"

/** @deprecated Use input_read(). */
#define input_get() \
    ZGDK_DEPRECATED_WARNING_("input_get() is deprecated; use input_read()") \
    input_read()
