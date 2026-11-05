#pragma once
#include <stdint.h>
#define DMA_DESC_LAST 1
#define VID_MEM_SPRITE_ADDR 0x102800UL
extern uint8_t zvb_config_dev_idx;
typedef union { uint8_t raw; } dma_flags_t;
#pragma pack(push, 1)
typedef struct {
    uint16_t rd_addr_lo; uint8_t rd_addr_hi;
    uint16_t wr_addr_lo; uint8_t wr_addr_hi;
    uint16_t length; dma_flags_t flags; uint8_t reserved[3];
} zvb_dma_descriptor_t;
#pragma pack(pop)
uint8_t zvb_dma_set_read_virt(zvb_dma_descriptor_t *, void *);
uint8_t zvb_dma_set_write_virt(zvb_dma_descriptor_t *, void *);
uint8_t zvb_dma_set_write(zvb_dma_descriptor_t *, uint32_t);
uint8_t zvb_dma_start_transfer(zvb_dma_descriptor_t *);

uint32_t zvb_dma_virt_to_phys(void *);
