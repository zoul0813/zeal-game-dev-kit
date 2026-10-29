# ZGDK: File

## file_read_safe

```c
#include "zgdk/file.h"

zos_err_t file_read_safe(zos_dev_t dev, void *buffer, uint16_t *size);
```

ZOS `read()` requires its buffer to stay inside one 16 KiB virtual page (see `zos_vfs.h`): a buffer that
straddles a page boundary, or is bigger than a page, can't be read in one call. `file_read_safe()` reads in
pieces that each stay inside one page, so any buffer in RAM works, whatever its address or size.

It has the same contract as `read()`: `*size` is the number of bytes wanted, and on return the number read.
It stops early, returning `ERR_SUCCESS`, at the end of the file. On an error it returns the error, with
`*size` set to the bytes read before it.

```c
zos_dev_t dev = open("level1.bin", O_RDONLY);
uint16_t size = sizeof(level);
zos_err_t err = file_read_safe(dev, level, &size);
close(dev);
if (err != ERR_SUCCESS || size != sizeof(level)) { /* missing or short file */ }
```

Use it for any read whose buffer might cross a page: level data, music or tracker files, save files. Small reads
into a buffer you know is inside one page can call `read()` directly.
