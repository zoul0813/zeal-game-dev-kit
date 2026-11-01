# Contributing

## Assembly formatting

Use four spaces, not tabs, to indent assembly instructions and assembler
directives. Keep labels at column one. Indent comments within instruction
blocks by four spaces as well.

```asm
    .area _TEXT
read_next_bit:
    in a,(0xd0)
    ret
```

## Deprecated APIs

Keep deprecated aliases in each namespace's own `deprecated.h`, included
automatically by its namespace header after the canonical declarations. Existing
callers must not need an extra include. Emit a notice when a deprecated alias is
used, not merely when its header is included, and verify diagnostics with SDCC.
Use `zgdk/deprecation.h` for the shared warning helper. SDK implementation code
should call the canonical APIs.
