#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zvb_dma.h>
#include "../../src/sprites/internal.h"

gfx_sprite *sprites_arena;
uint8_t sprites_arena_size, sprites_registered, sprites_rendered, zvb_config_dev_idx;
uint8_t sprites_dma_upload(uint8_t start, uint8_t count);
static uint8_t hardware[1024], *memory;
static uint8_t available;
static unsigned transfers, read_setups, write_setups;
static uint32_t mapping[2];
static struct {
    zvb_dma_descriptor_t *desc;
    void *source, *destination;
    uint8_t physical;
} records[65];

static unsigned record(zvb_dma_descriptor_t *desc) {
    for (unsigned i = 0; i < 65; ++i) {
        if (!records[i].desc) records[i].desc = desc;
        if (records[i].desc == desc) return i;
    }
    assert(!"Too many DMA descriptors");
    return 0;
}
uint32_t zvb_dma_virt_to_phys(void *ptr) {
    uintptr_t offset = (uintptr_t)ptr - (uintptr_t)memory;
    assert(offset < 0x8000);
    return 0x20000 + offset + mapping[offset / 0x4000];
}
uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t *desc, void *ptr) {
    records[record(desc)].source = ptr;
    /* Record the base address so uploads can temporarily offset a template. */
    uintptr_t address = (uintptr_t)ptr;
    desc->rd_addr_lo = address & 0xffff;
    desc->rd_addr_hi = (address >> 16) & 0xff;
    ++read_setups;
    return 0;
}
uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t *desc, void *ptr) {
    unsigned i = record(desc);
    records[i].destination = ptr;
    records[i].physical = 0;
    return 0;
}
uint8_t zvb_dma_set_write(zvb_dma_descriptor_t *desc, uint32_t address) {
    assert(address >= VID_MEM_SPRITE_ADDR && address < VID_MEM_SPRITE_ADDR + 1024);
    desc->wr_addr_lo = address & 0xffff;
    desc->wr_addr_hi = address >> 16;
    records[record(desc)].physical = 1;
    ++write_setups;
    return 0;
}
uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t *desc) {
    unsigned i = record(desc);
    uint16_t base = (uintptr_t)records[i].source & 0xffff;
    uint16_t skip = desc->rd_addr_lo - base;
    void *source = (uint8_t *)records[i].source + skip;
    zvb_config_dev_idx = 4;
    assert(((uintptr_t)desc & 0x3fff) + sizeof(*desc) <= 0x4000);
    assert(desc->flags.raw == DMA_DESC_LAST);
    assert(desc->length > 0 && desc->length <= SPRITES_DMA_CHUNK_BYTES);
    assert(((uintptr_t)source & 0x3fff) + desc->length <= 0x4000);
    if (available) {
        if (records[i].physical) {
            unsigned offset = desc->wr_addr_lo - 0x2800;
            assert(desc->wr_addr_hi == 0x10);
            assert(offset + desc->length <= sizeof(hardware));
            memcpy(hardware + offset, source, desc->length);
            ++transfers;
        } else memcpy(records[i].destination, source, desc->length);
    }
    return 0;
}
static void check_ranges(void) {
    unsigned prepared = read_setups + write_setups;
    for (unsigned start = 0; start < GFX_SPRITES_COUNT; ++start) {
        for (unsigned count = 1; count <= GFX_SPRITES_COUNT - start; ++count) {
            memset(hardware, 0xa5, sizeof(hardware));
            assert(sprites_dma_upload(start, count));
            assert(zvb_config_dev_idx == 7);
            unsigned first = start * sizeof(gfx_sprite);
            unsigned end = first + count * sizeof(gfx_sprite);
            assert(!memcmp(hardware + first, (uint8_t *)sprites_arena + first, end - first));
            for (unsigned i = 0; i < first; ++i) assert(hardware[i] == 0xa5);
            for (unsigned i = end; i < sizeof(hardware); ++i) assert(hardware[i] == 0xa5);
        }
    }
    assert(read_setups + write_setups == prepared); /* Templates remain reusable. */
}

int main(int argc, char **argv) {
    (void)argv;
    available = argc == 1;
    memory = aligned_alloc(0x4000, 0x8000);
    assert(memory);
    /* First sprite ends at a page boundary; later sprites use the next page. */
    sprites_arena = (void *)(memory + 0x3ff8);
    for (unsigned i = 0; i < sizeof(hardware); ++i)
        ((uint8_t *)sprites_arena)[i] = (i * 17 + 3) & 255;
    sprites_arena_size = sprites_registered = 128;
    zvb_config_dev_idx = 7;
    assert(zvb_config_dev_idx == 7 && transfers == 0 && read_setups == 0);
    assert(sprites_dma_upload(0, 128) == available);
    assert(zvb_config_dev_idx == 7);
    if (available) {
        unsigned prepared = read_setups + write_setups;
        assert(transfers == 1 + (1016 + SPRITES_DMA_CHUNK_BYTES - 1) / SPRITES_DMA_CHUNK_BYTES);
        assert(!memcmp(hardware, sprites_arena, sizeof(hardware)));
        /* Sprite contents change without rebuilding descriptors. */
        ((uint8_t *)sprites_arena)[0] ^= 255;
        assert(sprites_dma_upload(0, 128));
        assert(!memcmp(hardware, sprites_arena, sizeof(hardware)));
        /* Two sprites end partway through the second descriptor. */
        sprites_registered = 2;
        memset(hardware, 0xa5, sizeof(hardware));
        assert(sprites_dma_upload(0, 2));
        assert(!memcmp(hardware, sprites_arena, 16) && hardware[16] == 0xa5);
        sprites_registered = 128;
        assert(sprites_dma_upload(0, 128));
        assert(!memcmp(hardware, sprites_arena, sizeof(hardware)));
        assert(read_setups + write_setups == prepared);
        check_ranges();
        /* Shrinking cleans old slots using the same templates. */
        sprites_registered = 1;
        sprites_rendered = 128;
        memset((uint8_t *)sprites_arena + 8, 0, 1016);
        assert(sprites_dma_upload(0, 128));
        assert(!memcmp(hardware, sprites_arena, sizeof(hardware)));
        assert(read_setups + write_setups == prepared);
        /* Both source page mappings independently invalidate the cache. */
        mapping[0] = 0x40000;
        assert(sprites_dma_upload(3, 6));
        assert(read_setups + write_setups > prepared);
        prepared = read_setups + write_setups;
        mapping[1] = 0x80000;
        assert(sprites_dma_upload(1, 1));
        assert(read_setups + write_setups > prepared);
        check_ranges();
        /* An unaligned arena may split a sprite across physical pages. */
        sprites_arena = (void *)(memory + 0x3ffd);
        sprites_arena_size = sprites_registered = 128;
        sprites_rendered = 0;
        for (unsigned i = 0; i < sizeof(hardware); ++i)
            ((uint8_t *)sprites_arena)[i] = (i * 19 + 7) & 255;
        assert(sprites_dma_upload(0, 1)); /* Changed arena prepares lazily. */
        check_ranges();
        /* Replacing an arena prepares a new, smaller plan. */
        sprites_arena = (void *)(memory + 64);
        sprites_arena_size = sprites_registered = 1;
        sprites_rendered = 0;
        memset(sprites_arena, 0x39, sizeof(gfx_sprite));
        assert(sprites_dma_upload(0, 1));
        assert(!memcmp(hardware, sprites_arena, sizeof(gfx_sprite)));
        assert(zvb_config_dev_idx == 7);
        /* Resizing the same arena must rebuild its plan before a larger upload. */
        sprites_arena_size = sprites_registered = 128;
        prepared = read_setups + write_setups;
        assert(sprites_dma_upload(0, 128));
        assert(read_setups + write_setups > prepared);
        assert(!memcmp(hardware, sprites_arena, sizeof(hardware)));
    } else {
        unsigned probes = read_setups;
        assert(!sprites_dma_upload(3, 6));
        assert(transfers == 0 && read_setups == probes);
    }
    free(memory);
    puts("sprite DMA: reusable descriptors, partial chunks, source page changes, arena replacement, cleanup and absent controller passed");
}
