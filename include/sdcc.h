#ifndef SDCC_H
#define SDCC_H

#ifdef __SDCC_VERSION_MAJOR
#define __attribute__(a)
#else
typedef uint8_t zos_err_t;
#endif

#endif