/* Host test for file_read_safe(): every read() stays inside one 16 KiB page. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zos_vfs.h>
#include "zgdk/file.h"

_Alignas(0x4000) static uint8_t ram[0x10000];   // stands for the 64 KiB address space, page aligned
static uint8_t file[0x9000];
static uint16_t file_len, cursor;
static int reads, fail_at = -1;

zos_err_t read(zos_dev_t dev, void *buf, uint16_t *size)
{
    uintptr_t at = (uintptr_t)((uint8_t *)buf - ram);
    assert(dev == 7);
    assert(*size > 0 && *size <= 0x4000);
    assert(at / 0x4000 == (at + *size - 1) / 0x4000);   // the buffer doesn't cross a page
    if (reads++ == fail_at) return ERR_FAILURE;
    if (*size > file_len - cursor) *size = file_len - cursor;
    memcpy(buf, file + cursor, *size);
    cursor += *size;
    return ERR_SUCCESS;
}

static zos_err_t run(uint16_t at, uint16_t size, uint16_t *got)
{
    *got = size;
    cursor = 0;
    reads = 0;
    memset(ram, 0, sizeof(ram));
    return file_read_safe(7, ram + at, got);
}

int main(void)
{
    uint16_t got;
    for (unsigned i = 0; i < sizeof(file); i++) file[i] = (uint8_t)(i * 7 + 3);
    file_len = sizeof(file);

    // inside one page: one read
    assert(run(0x4100, 0x800, &got) == ERR_SUCCESS && got == 0x800 && reads == 1);
    assert(memcmp(ram + 0x4100, file, 0x800) == 0);

    // a 2 KiB buffer at 0x7C00 crosses into 0x8000: two reads, all bytes in order
    assert(run(0x7C00, 0x800, &got) == ERR_SUCCESS && got == 0x800 && reads == 2);
    assert(memcmp(ram + 0x7C00, file, 0x800) == 0);

    // two pages and a bit, starting one byte before a page: 1 + 0x4000 + 0x4000 + 1 bytes, four reads
    assert(run(0x3FFF, 0x8002, &got) == ERR_SUCCESS && got == 0x8002 && reads == 4);
    assert(memcmp(ram + 0x3FFF, file, 0x8002) == 0);

    // the file ends partway: the bytes there were, success, no read past the end
    file_len = 0x500;
    assert(run(0x7C00, 0x800, &got) == ERR_SUCCESS && got == 0x400 + 0x100 && reads == 2);
    file_len = 0x300;
    assert(run(0x7C00, 0x800, &got) == ERR_SUCCESS && got == 0x300 && reads == 1);
    file_len = sizeof(file);

    // a read error after the first page: the error, and the bytes read before it
    fail_at = 1;
    assert(run(0x7C00, 0x800, &got) == ERR_FAILURE && got == 0x400);
    fail_at = -1;

    // nothing asked, nothing read
    assert(run(0x1000, 0, &got) == ERR_SUCCESS && got == 0 && reads == 0);

    puts("file_read_safe: ok");
    return 0;
}
