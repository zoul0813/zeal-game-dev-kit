#pragma once

/* SDCC 4.4 does not support C11 function-deprecation attributes. Its bundled
 * sdcpp preprocessor DOES implement the "GCC warning" pragma used below; the
 * directive name does not require compiling with GCC. Verified with SDCC 4.4.
 * Emit warnings when a macro alias is used, not when its header is included.
 */
#define ZGDK_DEPRECATED_PRAGMA_(directive) _Pragma(#directive)
#define ZGDK_DEPRECATED_WARNING_(message) \
    ZGDK_DEPRECATED_PRAGMA_(GCC warning message)
