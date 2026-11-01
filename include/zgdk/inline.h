#ifndef ZGDK_INLINE_H
#define ZGDK_INLINE_H

/* SDCC emits unused bodies for static inline helpers. Plain inline keeps the
 * expansion without those copies. Host tests also run without optimization,
 * where static linkage supplies a callable fallback. If SDCC cannot inline a
 * helper (or its address is needed), move its body to one non-inline .c
 * definition and leave a declaration in the header. */
#ifdef __SDCC
#define INLINE inline
#else
#define INLINE static inline
#endif

#endif
