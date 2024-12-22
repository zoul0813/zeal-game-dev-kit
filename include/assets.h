#ifndef ASSETS_H
#define ASSETS_H

#define INCLUDE_ASSET(name, path)        \
    __asm__("__" name "_start:\n"        \
            "    .incbin \"" path "\"\n" \
            "__" name "_end:\n")

#endif