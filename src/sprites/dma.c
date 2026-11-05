#include <stdint.h>
#include <string.h>
#include <zvb_dma.h>
#include "internal.h"

/* A page split can add one descriptor to the maximum-sized arena. */
#define DMA_DESCRIPTORS (GFX_SPRITES_COUNT * sizeof(gfx_sprite) / SPRITES_DMA_CHUNK_BYTES + 1)
/* At most one page boundary crosses this small allocation; allow skipped bytes. */
static uint8_t descriptor_storage[DMA_DESCRIPTORS * sizeof(zvb_dma_descriptor_t)
                                  + sizeof(zvb_dma_descriptor_t) - 1];
static zvb_dma_descriptor_t *descriptors[DMA_DESCRIPTORS];
static uint8_t dma_state; /* 0 untested, 1 unavailable, 2 available */
static volatile uint8_t probe_result;
static uint32_t arena_physical, next_page_physical;
static uint8_t *next_page;

static zvb_dma_descriptor_t *page_safe(uint8_t *storage)
{
    uint16_t page_left = 0x4000 - ((uintptr_t)storage & 0x3fff);
    if (page_left < sizeof(zvb_dma_descriptor_t)) storage += page_left;
    return (void *)storage;
}

void sprites_dma_prepare(void)
{
    zvb_dma_descriptor_t *desc = page_safe(descriptor_storage);
    uint8_t bank = zvb_config_dev_idx;
    if (!dma_state) {
        uint8_t source = 0xa5;
        memset(desc, 0, sizeof(*desc));
        desc->flags.raw = DMA_DESC_LAST;
        desc->length = 1;
        probe_result = 0;
        zvb_dma_set_read_virt(desc, &source);
        zvb_dma_set_write_virt(desc, (void *)&probe_result);
        zvb_dma_start_transfer(desc);
        dma_state = probe_result == source ? 2 : 1;
    }
    if (dma_state == 2) {
        uint16_t remaining = (uint16_t)sprites_arena_size * sizeof(gfx_sprite);
        uint16_t offset = 0;
        uint8_t index = 0;
        uint8_t *source = (void *)sprites_arena;
        uint16_t page_left = 0x4000 - ((uintptr_t)source & 0x3fff);
        arena_physical = zvb_dma_virt_to_phys(source);
        next_page = remaining > page_left ? source + page_left : NULL;
        if (next_page) next_page_physical = zvb_dma_virt_to_phys(next_page);
        while (remaining) {
            page_left = 0x4000 - ((uintptr_t)source & 0x3fff);
            uint8_t length = remaining > SPRITES_DMA_CHUNK_BYTES ? SPRITES_DMA_CHUNK_BYTES : remaining;
            if (page_left < length) length = page_left;
            memset(desc, 0, sizeof(*desc));
            desc->flags.raw = DMA_DESC_LAST;
            desc->length = length;
            zvb_dma_set_read_virt(desc, source);
            zvb_dma_set_write(desc, VID_MEM_SPRITE_ADDR + offset);
            descriptors[index++] = desc;
            source += length;
            offset += length;
            remaining -= length;
            if (remaining) desc = page_safe((uint8_t *)desc + sizeof(*desc));
        }
    }
    zvb_config_dev_idx = bank;
}

uint8_t sprites_dma_upload(void)
{
    if (dma_state != 2) return 0;
    /* An arena may occupy two discontiguous physical pages. Rebuild only if
     * their mappings change; the descriptor API translates each descriptor. */
    if (zvb_dma_virt_to_phys(sprites_arena) != arena_physical ||
        (next_page && zvb_dma_virt_to_phys(next_page) != next_page_physical))
        sprites_dma_prepare();
    uint8_t count = sprites_registered > sprites_rendered ? sprites_registered : sprites_rendered;
    uint16_t remaining = (uint16_t)count * sizeof(gfx_sprite);
    uint8_t bank = zvb_config_dev_idx;
    uint8_t index = 0;
    while (remaining) {
        zvb_dma_descriptor_t *desc = descriptors[index++];
        uint16_t length = desc->length;
        if (remaining < length) {
            /* A partial final chunk must not change the reusable template. */
            desc->length = remaining;
            zvb_dma_start_transfer(desc);
            desc->length = length;
            break;
        }
        /* Separate last-marked transfers release the bus between chunks. */
        zvb_dma_start_transfer(desc);
        remaining -= length;
    }
    zvb_config_dev_idx = bank;
    return 1;
}
