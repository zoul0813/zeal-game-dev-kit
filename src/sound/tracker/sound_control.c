#include <zvb_sound.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

const __sfr __banked __at(0xF0) mmu_page0_ro;

void zmt_sound_off(void)
{
    uint8_t backup_page = mmu_page0_ro;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    SOUND_OFF();
    zvb_map_peripheral(backup_page);
}

/* reset the sound system, setting volume */
void zmt_reset(sound_volume_t vol)
{
    uint8_t backup_page = mmu_page0_ro;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    SOUND_RESET(vol);
    zvb_map_peripheral(backup_page);
}
