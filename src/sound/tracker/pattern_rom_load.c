#include <core.h>
#include "zgdk/sound/tracker.h"

zos_err_t zmt_pattern_rom_load(pattern_t* pattern, uint8_t **start)
{
    uint8_t *pos = *start;
    uint16_t size = 0;
    zos_err_t err = ERR_SUCCESS;
    uint8_t i, j;
    voice_t* voice;
    step_t* step;
    uint8_t voice_bitmap  = 0x00;
    uint32_t voice_header = 0x00;
    uint8_t step_header   = 0x00;
    uint8_t bit           = 0x00;

    if(pattern == NULL) return ERR_INVALID_OFFSET;

    zmt_pattern_init(pattern);

    /** Voice Bitmap */
    size = sizeof(uint8_t);
    mem_cpy(&voice_bitmap, pos, size);
    pos += size;

    // printf("  Voices: %02x\n", voice_bitmap);
    for (i = 0; i < NUM_VOICES; i++) {
        voice        = &pattern->voices[i];
        bit          = voice_bitmap & 0x01;
        voice_bitmap = voice_bitmap >> 1;
        if (!bit)
            continue; // empty voice
        // printf("  Voice: %d\n", i);

        /** Voice Header */
        size = sizeof(uint32_t);
        mem_cpy(&voice_header, pos, size);
        pos += size;

        for (j = 0; j < STEPS_PER_PATTERN; j++) {
            step         = &voice->steps[j];
            bit          = voice_header & 0x01;
            voice_header = voice_header >> 1;
            if (!bit)
                continue; // empty step

            size = sizeof(uint8_t);
            mem_cpy(&step_header, pos, size);
            pos += size;

            if (step_header & STEP_CELL_NOTE) {
                size = sizeof(note_index_t);
                mem_cpy(&step->note, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_WAVE) {
                size = sizeof(waveform_t);
                mem_cpy(&step->waveform, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_FX1) {
                size = sizeof(fx_t);
                mem_cpy(&step->fx1, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_FX2) {
                size = sizeof(fx_t);
                mem_cpy(&step->fx2, pos, size);
                pos += size;
            }
        }
    }

    *start = pos;
    return err;
}
