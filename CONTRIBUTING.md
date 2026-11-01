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
