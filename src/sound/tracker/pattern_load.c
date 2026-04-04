#include "zgdk/sound/tracker.h"
#include "private.h"

zos_err_t zmt_pattern_load(pattern_t* pattern, zos_dev_t dev)
{
    zmt_pattern_init(pattern);
    uint16_t size = 0;
    zos_err_t err = ERR_SUCCESS;
    uint8_t i, j;
    voice_t* voice;
    step_t* step;
    uint8_t voice_bitmap  = 0x00;
    uint32_t voice_header = 0x00;
    uint8_t step_header   = 0x00;
    uint8_t bit           = 0x00;

    i;
    j;
    voice;
    step;
    pattern;

    /** Voice Bitmap */
    size = sizeof(uint8_t);
    err  = read(dev, &voice_bitmap, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("read", 1, err);
        return err;
    }

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
        err  = read(dev, &voice_header, &size);
        if (err != ERR_SUCCESS) {
            tracker_log_io_error("read", 2, err);
            return err;
        }
        // printf("  Header: %08lx\n", voice_header);

        for (j = 0; j < STEPS_PER_PATTERN; j++) {
            step         = &voice->steps[j];
            bit          = voice_header & 0x01;
            voice_header = voice_header >> 1;
            if (!bit)
                continue; // empty step

            size = sizeof(uint8_t);
            err  = read(dev, &step_header, &size);
            if (err != ERR_SUCCESS) {
                tracker_log_io_error("read", 3, err);
                return err;
            }

            if (step_header & STEP_CELL_NOTE) {
                size = sizeof(note_index_t);
                err  = read(dev, &step->note, &size);
                if (err != ERR_SUCCESS) {
                    tracker_log_io_error("read", 4, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_WAVE) {
                size = sizeof(waveform_t);
                err  = read(dev, &step->waveform, &size);
                if (err != ERR_SUCCESS) {
                    tracker_log_io_error("read", 5, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_FX1) {
                size = sizeof(fx_t);
                err  = read(dev, &step->fx1, &size);
                if (err != ERR_SUCCESS) {
                    tracker_log_io_error("read", 6, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_FX2) {
                size = sizeof(fx_t);
                err  = read(dev, &step->fx2, &size);
                if (err != ERR_SUCCESS) {
                    tracker_log_io_error("read", 7, err);
                    return err;
                }
            }
        }
    }

    return err;
}
