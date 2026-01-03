#ifndef SDCC_H
#define SDCC_H

// intellisense trick for vscode
#ifndef __SDCC_VERSION_MAJOR
#define __at(addr)
#define __sfr
#define __banked
#define __naked
#endif
#ifdef __SDCC_VERSION_MAJOR
#define __attribute__(a)
#endif

#endif
