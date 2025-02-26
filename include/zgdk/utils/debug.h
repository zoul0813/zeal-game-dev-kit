#ifndef DEBUG_H
#define DEBUG_H

/**
 * TSTATE_LOG will write `counter` to the "debug_register" at 0x86
 *
 * @param counter The value to log
 */
#ifdef EMULATOR
__sfr __at(0x86) debug_register; // t-state counter
#define TSTATE_LOG(counter) debug_register = counter;
#else
#define TSTATE_LOG(counter)
#endif

#endif