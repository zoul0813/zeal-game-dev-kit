#include "zgdk/sound/tracker.h"
#include "private.h"

#define STEP_IS_EMPTY(step)                                                            \
    ((step->note == NOTE_OUT_OF_RANGE) && (step->waveform == WAVEFORM_OUT_OF_RANGE) && \
     (step->fx1 == FX_OUT_OF_RANGE) && (step->fx2 == FX_OUT_OF_RANGE))

/* save a pattern to file */
zos_err_t zmt_pattern_save(pattern_t* pattern, zos_dev_t dev)
{
    uint16_t size = 0;
    zos_err_t err = ERR_SUCCESS;
    uint8_t i = 0, j = 0;
    voice_t* voice;
    step_t* step;
    uint8_t voice_bitmap      = 0x00;
    uint32_t voice_headers[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t step_header       = 0x00;
    uint8_t bit               = 0x00;

    /** Voice Bitmap + voice_header */
    for (i = NUM_VOICES; i > 0; i--) {
        voice = &pattern->voices[i - 1];
        for (j = STEPS_PER_PATTERN; j > 0; j--) {
            step                 = &voice->steps[j - 1];
            voice_headers[i - 1] = voice_headers[i - 1] << 1;
            if (!STEP_IS_EMPTY(step))
                voice_headers[i - 1]++;
        }
        voice_bitmap = voice_bitmap << 1;
        if (voice_headers[i - 1] > 0)
            voice_bitmap++;
    }

    size = sizeof(uint8_t);
    err  = write(dev, &voice_bitmap, &size);
    if (err != ERR_SUCCESS) {
        tracker_log_io_error("write", 1, err);
        return err;
    }

    for (i = 0; i < NUM_VOICES; i++) {
        if (voice_headers[i] == 0x00)
            continue;

        voice = &pattern->voices[i];

        size = sizeof(uint32_t);
        write(dev, &voice_headers[i], &size);
        if (err != ERR_SUCCESS) {
            tracker_log_io_error("write", 2, err);
            return err;
        }

        for (j = 0; j < STEPS_PER_PATTERN; j++) {
            step             = &voice->steps[j];
            bit              = voice_headers[i] & 0x01;
            voice_headers[i] = voice_headers[i] >> 1;

            if (bit) {
                step_header = 0x00;
                if (step->note != NOTE_OUT_OF_RANGE)
                    step_header |= STEP_CELL_NOTE;
                if (step->waveform != WAVEFORM_OUT_OF_RANGE)
                    step_header |= STEP_CELL_WAVE;
                if (step->fx1 != FX_OUT_OF_RANGE)
                    step_header |= STEP_CELL_FX1;
                if (step->fx2 != FX_OUT_OF_RANGE)
                    step_header |= STEP_CELL_FX2;

                size = sizeof(uint8_t);
                err  = write(dev, &step_header, &size);
                if (err != ERR_SUCCESS) {
                    tracker_log_io_error("write", 3, err);
                    return err;
                }

                if (step->note != NOTE_OUT_OF_RANGE) {
                    size = sizeof(note_index_t);
                    write(dev, &step->note, &size);
                }
                if (step->waveform != WAVEFORM_OUT_OF_RANGE) {
                    size = sizeof(waveform_t);
                    write(dev, &step->waveform, &size);
                }
                if (step->fx1 != FX_OUT_OF_RANGE) {
                    size = sizeof(fx_t);
                    write(dev, &step->fx1, &size);
                }
                if (step->fx2 != FX_OUT_OF_RANGE) {
                    size = sizeof(fx_t);
                    write(dev, &step->fx2, &size);
                }
            }
        }
    }
    return err;
}
