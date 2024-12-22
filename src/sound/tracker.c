#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <zvb_sound.h>
#include "sound/tracker.h"

static uint8_t frames              = 0;
static uint8_t current_pattern     = 0;
static uint8_t current_arrangement = 0;
static uint8_t next_step           = 0;
static uint8_t last_step           = 0;

static const char* str_error_msg_write = "failed to write %d, %0d (%02x)\n";
static const char* str_error_msg_read  = "failed to write %d, %0d (%02x)\n";

/** Page Banking for Sound Peripheral */
const __sfr __banked __at(0xF0) mmu_page0_ro;

#define STEP_IS_EMPTY(step)                                                            \
    ((step->note == NOTE_OUT_OF_RANGE) && (step->waveform == WAVEFORM_OUT_OF_RANGE) && \
     (step->fx1 == FX_OUT_OF_RANGE) && (step->fx2 == FX_OUT_OF_RANGE))

#define ARRANGEMENT_IS_EMPTY(arrange) \
    ((arrange->pattern_index == ARRANGEMENT_OUT_OF_RANGE) && (arrange->fx == FX_OUT_OF_RANGE))

note_t NOTES[] = {
    /* Octave 0 */
    SOUND_FREQ_TO_DIV(16), // C0
    SOUND_FREQ_TO_DIV(17), // C#0
    SOUND_FREQ_TO_DIV(18), // D0
    SOUND_FREQ_TO_DIV(19), // D#0
    SOUND_FREQ_TO_DIV(21), // E0
    SOUND_FREQ_TO_DIV(22), // F0
    SOUND_FREQ_TO_DIV(23), // F#0
    SOUND_FREQ_TO_DIV(24), // G0
    SOUND_FREQ_TO_DIV(26), // G#0
    SOUND_FREQ_TO_DIV(27), // A0
    SOUND_FREQ_TO_DIV(29), // A#0
    SOUND_FREQ_TO_DIV(31), // B0

    /* Octave 1 */
    SOUND_FREQ_TO_DIV(33), // C1
    SOUND_FREQ_TO_DIV(35), // C#1
    SOUND_FREQ_TO_DIV(37), // D1
    SOUND_FREQ_TO_DIV(39), // D#1
    SOUND_FREQ_TO_DIV(41), // E1
    SOUND_FREQ_TO_DIV(44), // F1
    SOUND_FREQ_TO_DIV(47), // F#1
    SOUND_FREQ_TO_DIV(49), // G1
    SOUND_FREQ_TO_DIV(52), // G#1
    SOUND_FREQ_TO_DIV(55), // A1
    SOUND_FREQ_TO_DIV(59), // A#1
    SOUND_FREQ_TO_DIV(62), // B1

    /* Octave 2 */
    SOUND_FREQ_TO_DIV(65),  // C2
    SOUND_FREQ_TO_DIV(69),  // C#2
    SOUND_FREQ_TO_DIV(73),  // D2
    SOUND_FREQ_TO_DIV(78),  // D#2
    SOUND_FREQ_TO_DIV(82),  // E2
    SOUND_FREQ_TO_DIV(87),  // F2
    SOUND_FREQ_TO_DIV(92),  // F#2
    SOUND_FREQ_TO_DIV(98),  // G2
    SOUND_FREQ_TO_DIV(104), // G#2
    SOUND_FREQ_TO_DIV(110), // A2
    SOUND_FREQ_TO_DIV(117), // A#2
    SOUND_FREQ_TO_DIV(123), // B2

    /* Octave 3 */
    SOUND_FREQ_TO_DIV(131), // C3
    SOUND_FREQ_TO_DIV(139), // C#3
    SOUND_FREQ_TO_DIV(147), // D3
    SOUND_FREQ_TO_DIV(156), // D#3
    SOUND_FREQ_TO_DIV(165), // E3
    SOUND_FREQ_TO_DIV(175), // F3
    SOUND_FREQ_TO_DIV(185), // F#3
    SOUND_FREQ_TO_DIV(196), // G3
    SOUND_FREQ_TO_DIV(208), // G#3
    SOUND_FREQ_TO_DIV(220), // A3
    SOUND_FREQ_TO_DIV(233), // A#3
    SOUND_FREQ_TO_DIV(247), // B3

    /* Octave 4 */
    SOUND_FREQ_TO_DIV(262), // C4
    SOUND_FREQ_TO_DIV(277), // C#4
    SOUND_FREQ_TO_DIV(294), // D4
    SOUND_FREQ_TO_DIV(311), // D#4
    SOUND_FREQ_TO_DIV(330), // E4
    SOUND_FREQ_TO_DIV(349), // F4
    SOUND_FREQ_TO_DIV(370), // F#4
    SOUND_FREQ_TO_DIV(392), // G4
    SOUND_FREQ_TO_DIV(415), // G#4
    SOUND_FREQ_TO_DIV(440), // A4
    SOUND_FREQ_TO_DIV(466), // A#4
    SOUND_FREQ_TO_DIV(494), // B4

    /* Octave 5 */
    SOUND_FREQ_TO_DIV(523), // C5
    SOUND_FREQ_TO_DIV(554), // C#5
    SOUND_FREQ_TO_DIV(587), // D5
    SOUND_FREQ_TO_DIV(622), // D#5
    SOUND_FREQ_TO_DIV(659), // E5
    SOUND_FREQ_TO_DIV(698), // F5
    SOUND_FREQ_TO_DIV(740), // F#5
    SOUND_FREQ_TO_DIV(784), // G5
    SOUND_FREQ_TO_DIV(830), // G#5
    SOUND_FREQ_TO_DIV(880), // A5
    SOUND_FREQ_TO_DIV(932), // A#5
    SOUND_FREQ_TO_DIV(988), // B5

    /* Octave 6 */
    SOUND_FREQ_TO_DIV(1046), // C6
    SOUND_FREQ_TO_DIV(1109), // C#6
    SOUND_FREQ_TO_DIV(1175), // D6
    SOUND_FREQ_TO_DIV(1244), // D#6
    SOUND_FREQ_TO_DIV(1319), // E6
    SOUND_FREQ_TO_DIV(1397), // F6
    SOUND_FREQ_TO_DIV(1480), // F#6
    SOUND_FREQ_TO_DIV(1568), // G6
    SOUND_FREQ_TO_DIV(1661), // G#6
    SOUND_FREQ_TO_DIV(1760), // A6
    SOUND_FREQ_TO_DIV(1865), // A#6
    SOUND_FREQ_TO_DIV(1975), // B6

    /* Octave 7 */
    SOUND_FREQ_TO_DIV(2093), // C7
    SOUND_FREQ_TO_DIV(2217), // C#7
    SOUND_FREQ_TO_DIV(2349), // D7
    SOUND_FREQ_TO_DIV(2489), // D#7
    SOUND_FREQ_TO_DIV(2637), // E7
    SOUND_FREQ_TO_DIV(2794), // F7
    SOUND_FREQ_TO_DIV(2960), // F#7
    SOUND_FREQ_TO_DIV(3136), // G7
    SOUND_FREQ_TO_DIV(3322), // G#7
    SOUND_FREQ_TO_DIV(3520), // A7
    SOUND_FREQ_TO_DIV(3729), // A#7
    SOUND_FREQ_TO_DIV(3951), // B7

    /* Octave 8 */
    SOUND_FREQ_TO_DIV(4186), // C8
    SOUND_FREQ_TO_DIV(4435), // C#8
    SOUND_FREQ_TO_DIV(4698), // D8
    SOUND_FREQ_TO_DIV(4978), // D#8
    SOUND_FREQ_TO_DIV(5274), // E8
    SOUND_FREQ_TO_DIV(5588), // F8
    SOUND_FREQ_TO_DIV(5920), // F#8
    SOUND_FREQ_TO_DIV(6272), // G8
    SOUND_FREQ_TO_DIV(6645), // G#8
    SOUND_FREQ_TO_DIV(7040), // A8
    SOUND_FREQ_TO_DIV(7459), // A#8
    SOUND_FREQ_TO_DIV(7902), // B8

    0x0000, // OFF
};

note_name_t NOTE_NAMES[] = {
    {'C', '-', '0'},
    {'C', '#', '0'},
    {'D', '-', '0'},
    {'D', '#', '0'},
    {'E', '-', '0'},
    {'F', '-', '0'},
    {'F', '#', '0'},
    {'G', '-', '0'},
    {'G', '#', '0'},
    {'A', '-', '0'},
    {'A', '#', '0'},
    {'B', '-', '0'},

    {'C', '-', '1'},
    {'C', '#', '1'},
    {'D', '-', '1'},
    {'D', '#', '1'},
    {'E', '-', '1'},
    {'F', '-', '1'},
    {'F', '#', '1'},
    {'G', '-', '1'},
    {'G', '#', '1'},
    {'A', '-', '1'},
    {'A', '#', '1'},
    {'B', '-', '1'},

    {'C', '-', '2'},
    {'C', '#', '2'},
    {'D', '-', '2'},
    {'D', '#', '2'},
    {'E', '-', '2'},
    {'F', '-', '2'},
    {'F', '#', '2'},
    {'G', '-', '2'},
    {'G', '#', '2'},
    {'A', '-', '2'},
    {'A', '#', '2'},
    {'B', '-', '2'},

    {'C', '-', '3'},
    {'C', '#', '3'},
    {'D', '-', '3'},
    {'D', '#', '3'},
    {'E', '-', '3'},
    {'F', '-', '3'},
    {'F', '#', '3'},
    {'G', '-', '3'},
    {'G', '#', '3'},
    {'A', '-', '3'},
    {'A', '#', '3'},
    {'B', '-', '3'},

    {'C', '-', '4'},
    {'C', '#', '4'},
    {'D', '-', '4'},
    {'D', '#', '4'},
    {'E', '-', '4'},
    {'F', '-', '4'},
    {'F', '#', '4'},
    {'G', '-', '4'},
    {'G', '#', '4'},
    {'A', '-', '4'},
    {'A', '#', '4'},
    {'B', '-', '4'},

    {'C', '-', '5'},
    {'C', '#', '5'},
    {'D', '-', '5'},
    {'D', '#', '5'},
    {'E', '-', '5'},
    {'F', '-', '5'},
    {'F', '#', '5'},
    {'G', '-', '5'},
    {'G', '#', '5'},
    {'A', '-', '5'},
    {'A', '#', '5'},
    {'B', '-', '5'},

    {'C', '-', '6'},
    {'C', '#', '6'},
    {'D', '-', '6'},
    {'D', '#', '6'},
    {'E', '-', '6'},
    {'F', '-', '6'},
    {'F', '#', '6'},
    {'G', '-', '6'},
    {'G', '#', '6'},
    {'A', '-', '6'},
    {'A', '#', '6'},
    {'B', '-', '6'},

    {'C', '-', '7'},
    {'C', '#', '7'},
    {'D', '-', '7'},
    {'D', '#', '7'},
    {'E', '-', '7'},
    {'F', '-', '7'},
    {'F', '#', '7'},
    {'G', '-', '7'},
    {'G', '#', '7'},
    {'A', '-', '7'},
    {'A', '#', '7'},
    {'B', '-', '7'},

    {'C', '-', '8'},
    {'C', '#', '8'},
    {'D', '-', '8'},
    {'D', '#', '8'},
    {'E', '-', '8'},
    {'F', '-', '8'},
    {'F', '#', '8'},
    {'G', '-', '8'},
    {'G', '#', '8'},
    {'A', '-', '8'},
    {'A', '#', '8'},
    {'B', '-', '8'},

    {'O', 'F', 'F'}
};


/* Tick the playhead forward one frame, called every video frame */
uint8_t zmt_tick(track_t* track, uint8_t use_arrangement)
{
    pattern_t* pattern = track->patterns[current_pattern];
    if (frames > track->current_tempo)
        frames = 0;
    // if(frames % FRAMES_PER_EIGTH == 0) { }
    if (frames % FRAMES_PER_SIXTEENTH(track->current_tempo) == 0) {
        last_step = next_step;
        zmt_play_pattern(pattern, next_step);

        next_step++;
        if (next_step >= STEPS_PER_PATTERN) {
            if (use_arrangement) {
                current_arrangement++;
                arrangement_t* arrangement = &track->arrangement[current_arrangement];
                if (arrangement->pattern_index == ARRANGEMENT_OUT_OF_RANGE) {
                    // reset
                    current_arrangement = 0;
                    arrangement         = &track->arrangement[current_arrangement];
                }
                zmt_process_arrangement_fx(track, arrangement);
                current_pattern = arrangement->pattern_index;
            }
            next_step = 0;
        }
    }
    frames++;

    return next_step;
}

uint8_t zmt_track_get_arrangement(track_t* track)
{
    track; // unreferenced
    return current_arrangement;
}
uint8_t zmt_track_get_pattern(track_t* track)
{
    track; // unreferenced
    return current_pattern;
}
uint8_t zmt_track_get_next_step(track_t* track)
{
    track; // unreferenced
    return next_step;
}
uint8_t zmt_track_get_last_step(track_t* track)
{
    track; // unreferenced
    return last_step;
}
uint8_t zmt_track_get_frame(track_t* track)
{
    track; // unreferenced
    return frames;
}

void zmt_process_arrangement_fx(track_t* track, arrangement_t* arrangement)
{
    // NOTE: watch out for EVELYN, consecutive comparisons should account for logic
    // Check arrangement FX ...
    uint8_t value = arrangement->fx;
    if (LESSTHAN(value, 0x1F)) {
        current_arrangement = value;
        return;
    }
    if (LESSTHAN(value, 0x3F)) {
        value                = ((arrangement->fx - 0x20) + 1) * 4;
        track->current_tempo = value;
        return;
    }
    if (RANGE(value, 0xD0, 0xDC)) {
        value = (arrangement->fx - 0xD0);
        return;
    }
}

void zmt_process_fx(step_t* step, fx_t fx, sound_voice_t voice)
{
    (void*) voice; // TODO: per channel effects
    // pattern_t *pattern = active_pattern; // TODO: gonna have more than one pattern soon, lol

    if (RANGE(fx, FX_COUNT_0, FX_COUNT_8)) {
        if (step->fx1_attr == 0x00)
            step->fx1_attr = (fx - 0xC0);
        else
            step->fx1_attr--;
        return;
    }

    if(RANGE(fx, FX_GOTO_0, FX_GOTO_31)) {
        next_step = (fx - FX_GOTO_0) - 1;
        frames    = (next_step % 16) - 1; // TODO: yeah?
        return;
    }

    switch (fx) {
        case FX_NOTE_OFF: {
        } break;
        case FX_NOTE_ON: {
        } break;

        case FX_VOICE_SQ:  // fall thru
        case FX_VOICE_TRI: // fall thru
        case FX_VOICE_SAW: // fall thru
        case FX_VOICE_NOISE: {
            SOUND_WAVE(fx);
        } break;

        // TODO: per channel volume control?
        case FX_VOL_00_0: {
            SOUND_VOL(VOL_0);
        } break;
        case FX_VOL_12_5: {
            // TODO: 8 step volume control
            SOUND_VOL(VOL_0);
        } break;
        case FX_VOL_25_0: {
            SOUND_VOL(VOL_25);
        } break;
        case FX_VOL_37_5: {
            // TODO: 8 step volume control
            SOUND_VOL(VOL_25);
        } break;
        case FX_VOL_50_0: {
            SOUND_VOL(VOL_50);
        } break;
        case FX_VOL_62_5: {
            // TODO: 8 step volume control
            SOUND_VOL(VOL_50);
        } break;
        case FX_VOL_75_0: {
            SOUND_VOL(VOL_75);
        } break;
        case FX_VOL_87_5: {
            // TODO: 8 step volume control
            SOUND_VOL(VOL_75);
        } break;
        case FX_VOL_100: {
            SOUND_VOL(VOL_100);
        } break;
    }
}

void zmt_play_step(step_t* step, sound_voice_t voice)
{
    // pattern_t *pattern = active_pattern; // TODO: gonna have more than one pattern soon, lol

    // FX1 is pre-processed
    if (step->fx1 != FX_OUT_OF_RANGE)
        zmt_process_fx(step, step->fx1, voice);

    SOUND_SELECT(voice);

    if (step->note != NOTE_OUT_OF_RANGE) {
        note_t note = NOTES[step->note];
        SOUND_DIV(note);
    }

    if (step->waveform != WAVEFORM_OUT_OF_RANGE) {
        waveform_t waveform = step->waveform;
        if (waveform > 0x03)
            waveform = WAV_SQUARE;
        SOUND_WAVE(waveform);
    }

    // only process fx2 if fx1 is not counting, or it has counted down
    if ((step->fx1 >= FX_COUNT_0) && (step->fx1 <= FX_COUNT_8) && (step->fx1_attr != 0))
        return;
    // FX2 is post-processed ... does this matter?
    if (step->fx2 != FX_OUT_OF_RANGE)
        zmt_process_fx(step, step->fx2, voice);
}

void zmt_play_pattern(pattern_t* pattern, uint8_t step_index)
{
    step_t* step1 = &pattern->voices[0].steps[step_index];
    step_t* step2 = &pattern->voices[1].steps[step_index];
    step_t* step3 = &pattern->voices[2].steps[step_index];
    step_t* step4 = &pattern->voices[3].steps[step_index];

    uint8_t backup_page = mmu_page0_ro;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    zmt_play_step(step1, VOICE0);
    zmt_play_step(step2, VOICE1);
    zmt_play_step(step3, VOICE2);
    zmt_play_step(step4, VOICE3);
    zvb_map_peripheral(backup_page);
}


/* load a pattern from file */
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
    memcpy(&voice_bitmap, pos, size);
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
        memcpy(&voice_header, pos, size);
        pos += size;

        for (j = 0; j < STEPS_PER_PATTERN; j++) {
            step         = &voice->steps[j];
            bit          = voice_header & 0x01;
            voice_header = voice_header >> 1;
            if (!bit)
                continue; // empty step

            size = sizeof(uint8_t);
            memcpy(&step_header, pos, size);
            pos += size;

            if (step_header & STEP_CELL_NOTE) {
                size = sizeof(note_index_t);
                memcpy(&step->note, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_WAVE) {
                size = sizeof(waveform_t);
                memcpy(&step->waveform, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_FX1) {
                size = sizeof(fx_t);
                memcpy(&step->fx1, pos, size);
                pos += size;
            }

            if (step_header & STEP_CELL_FX2) {
                size = sizeof(fx_t);
                memcpy(&step->fx2, pos, size);
                pos += size;
            }
        }
    }

    *start = pos;
    return err;
}
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
        printf(str_error_msg_read, 1, err, err);
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
            printf(str_error_msg_read, 2, err, err);
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
                printf(str_error_msg_read, 3, err, err);
                return err;
            }

            if (step_header & STEP_CELL_NOTE) {
                size = sizeof(note_index_t);
                err  = read(dev, &step->note, &size);
                if (err != ERR_SUCCESS) {
                    printf(str_error_msg_read, 4, err, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_WAVE) {
                size = sizeof(waveform_t);
                err  = read(dev, &step->waveform, &size);
                if (err != ERR_SUCCESS) {
                    printf(str_error_msg_read, 5, err, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_FX1) {
                size = sizeof(fx_t);
                err  = read(dev, &step->fx1, &size);
                if (err != ERR_SUCCESS) {
                    printf(str_error_msg_read, 6, err, err);
                    return err;
                }
            }

            if (step_header & STEP_CELL_FX2) {
                size = sizeof(fx_t);
                err  = read(dev, &step->fx2, &size);
                if (err != ERR_SUCCESS) {
                    printf(str_error_msg_read, 7, err, err);
                    return err;
                }
            }
        }
    }

    return err;
}
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

    // printf("  Voices: %02x\n", voice_bitmap);
    size = sizeof(uint8_t);
    err  = write(dev, &voice_bitmap, &size);
    if (err != ERR_SUCCESS) {
        printf(str_error_msg_write, 1, err, err);
        return err;
    }

    for (i = 0; i < NUM_VOICES; i++) {
        // empty voice, skip it
        if (voice_headers[i] == 0x00)
            continue;

        voice = &pattern->voices[i];

        size = sizeof(uint32_t);
        write(dev, &voice_headers[i], &size);
        if (err != ERR_SUCCESS) {
            printf(str_error_msg_write, 2, err, err);
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

                // printf("  Voice %d, Step %d %02x\n", i, j, step_header);
                size = sizeof(uint8_t);
                err  = write(dev, &step_header, &size);
                if (err != ERR_SUCCESS) {
                    printf(str_error_msg_write, 3, err, err);
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
/* load arrange from file */
zos_err_t zmt_arrangement_rom_load(arrangement_t arrangement[NUM_ARRANGEMENTS], uint8_t **start)
{
    uint8_t *pos = *start;
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bit          = 0;

    size = sizeof(uint32_t);
    memcpy(&bitmap_low, pos, size);
    pos += size;

    size = sizeof(uint32_t);
    memcpy(&bitmap_high, pos, size);
    pos += size;


    for (i = 0; i < NUM_ARRANGEMENTS / 2; i++) {
        bit        = bitmap_low & 0x01;
        bitmap_low = bitmap_low >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        memcpy(a, pos, size);
        pos += size;
    }
    for (i = NUM_ARRANGEMENTS / 2; i < NUM_ARRANGEMENTS; i++) {
        bit         = bitmap_high & 0x01;
        bitmap_high = bitmap_high >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        memcpy(a, pos, size);
        pos += size;
    }

    *start = pos;
    return err;
}
zos_err_t zmt_arrangement_load(arrangement_t arrangement[NUM_ARRANGEMENTS], zos_dev_t dev)
{
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bit          = 0;

    size = sizeof(uint32_t);
    err  = read(dev, &bitmap_low, &size);
    if (err != ERR_SUCCESS) {
        printf(str_error_msg_write, 26, err, err);
        return err;
    }
    size = sizeof(uint32_t);
    err  = read(dev, &bitmap_high, &size);
    if (err != ERR_SUCCESS) {
        printf(str_error_msg_write, 27, err, err);
        return err;
    }

    for (i = 0; i < NUM_ARRANGEMENTS / 2; i++) {
        bit        = bitmap_low & 0x01;
        bitmap_low = bitmap_low >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        err  = read(dev, a, &size);
        if (err != ERR_SUCCESS) {
            printf(str_error_msg_write, 18, err, err);
            return err;
        }
    }
    for (i = NUM_ARRANGEMENTS / 2; i < NUM_ARRANGEMENTS; i++) {
        bit         = bitmap_high & 0x01;
        bitmap_high = bitmap_high >> 1;
        if (bit == 0)
            continue;
        a    = &arrangement[i];
        size = sizeof(arrangement_t);
        err  = read(dev, a, &size);
        if (err != ERR_SUCCESS) {
            printf(str_error_msg_write, 18, err, err);
            return err;
        }
    }

    return err;
}
/* save arrangement to file */
zos_err_t zmt_arrangement_save(arrangement_t arrangement[NUM_ARRANGEMENTS], zos_dev_t dev)
{
    zos_err_t err = ERR_SUCCESS;
    uint16_t size = 0;
    uint8_t i     = 0;
    arrangement_t* a;

    uint32_t bitmap_high = 0;
    uint32_t bitmap_low  = 0;
    uint8_t bitmap_bit   = 0;

    /* generate bitmap_low */
    for (i = NUM_ARRANGEMENTS; i > NUM_ARRANGEMENTS / 2; i--) {
        a           = &arrangement[i - 1];
        bitmap_high = bitmap_high << 1;
        if (!ARRANGEMENT_IS_EMPTY(a))
            bitmap_high++;
    }
    /* generate bitmap_high */
    for (i = NUM_ARRANGEMENTS / 2; i > 0; i--) {
        a          = &arrangement[i - 1];
        bitmap_low = bitmap_low << 1;
        if (!ARRANGEMENT_IS_EMPTY(a))
            bitmap_low++;
    }

    size = sizeof(uint32_t);
    err  = write(dev, &bitmap_low, &size);
    if (err != ERR_SUCCESS) {
        printf(str_error_msg_write, 16, err, err);
        return err;
    }

    size = sizeof(uint32_t);
    err  = write(dev, &bitmap_high, &size);
    if (err != ERR_SUCCESS) {
        printf(str_error_msg_write, 17, err, err);
        return err;
    }

    /* generate arrangement data */
    for (i = 0; i < NUM_ARRANGEMENTS; i++) {
        a = &arrangement[i];
        if (!ARRANGEMENT_IS_EMPTY(a)) {
            size = sizeof(arrangement_t);
            err  = write(dev, a, &size);
            if (err != ERR_SUCCESS) {
                printf(str_error_msg_write, 18, err, err);
                return err;
            }
        }
    }

    return err;
}
/* load a track from file*/
zos_err_t zmt_file_load(track_t* track, const char* filename)
{
    zos_err_t err;
    uint16_t size = 0;
    char textbuff[TRACKER_TITLE_LEN + 1];

    zmt_track_init(track); // clear out the track, before loading data

    zos_dev_t file_dev = open(filename, O_RDONLY);
    if (file_dev < 0) {
        printf("failed to open file, %d (%02x)\n", -file_dev, -file_dev);
        return -file_dev;
    }

    // printf("Loading '%s' ...\n", filename);

    size = 3;
    err  = read(file_dev, textbuff, &size); // format header
    if (err != ERR_SUCCESS) {
        printf("error reading format header, %d (%02x)\n", err, err);
        return err;
    }
    // printf("Format: %.3s\n", textbuff);

    size = sizeof(uint8_t);
    err  = read(file_dev, textbuff, &size); // version header
    if (err != ERR_SUCCESS) {
        printf("error reading version header, %d (%02x)\n", err, err);
        return err;
    }
    // printf("Version: %d\n", textbuff[0]);

    size = TRACKER_TITLE_LEN;
    err  = read(file_dev, textbuff, &size); // track title
    if (err != ERR_SUCCESS) {
        printf("error reading track title, %d (%02x)\n", err, err);
        return err;
    }
    memcpy(track->title, textbuff, size);
    track->title[TRACKER_TITLE_LEN] = 0x00; // NUL
    // printf("Track: %12s (read: %d)\n", track->title, size);

    size = sizeof(uint8_t);
    err  = read(file_dev, &track->tempo, &size);
    if (err != ERR_SUCCESS) {
        printf("error reading tempo, %d (%02x)\n", err, err);
        return err;
    }

    track->current_tempo = track->tempo;

    size = sizeof(uint8_t);
    err  = read(file_dev, &track->pattern_count, &size); // pattern count
    if (err != ERR_SUCCESS) {
        printf("error reading pattern count, %d (%02x)\n", err, err);
        return err;
    }
    // printf("Patterns: %d\n", track->pattern_count);

    err = zmt_arrangement_load(track->arrangement, file_dev);
    if (err != ERR_SUCCESS) {
        printf("error reading arrangement, %d (%02x)\n", err, err);
        return err;
    }

    for (uint8_t p = 0; p < track->pattern_count; p++) {
        // printf("Loading pattern %d\n", p);
        err = zmt_pattern_load(track->patterns[p], file_dev);
        if (err != ERR_SUCCESS) {
            printf("error loading patterns, %d (%02x)\n", err, err);
            return err;
        }
    }

    // printf("File loaded.\n\n");
    err = close(file_dev);
    return err;
}
/* save a track to file */
zos_err_t zmt_file_save(track_t* track, const char* filename)
{
    zos_err_t err;
    uint16_t size = 0;
    char textbuff[TRACKER_TITLE_LEN + 1];
    zos_dev_t file_dev = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (file_dev < 0) {
        printf("failed to open file for saving, '%s' %d (%02x)\n", filename, -file_dev, -file_dev);
        return -file_dev;
    }

    // printf("Saving '%s' ...\n", filename);

    /** FILE HEADER */
    size = 3;
    err  = write(file_dev, "ZMT", &size); // format header
    if (err != ERR_SUCCESS) {
        printf("error saving format header, %d (%02x)\n", err, err);
        return err;
    }

    size        = sizeof(uint8_t);
    textbuff[0] = 0;
    err         = write(file_dev, textbuff, &size); // version header
    if (err != ERR_SUCCESS) {
        printf("error saving version header, %d (%02x)\n", err, err);
        return err;
    }

    size = TRACKER_TITLE_LEN;
    sprintf(textbuff, "%-.12s", track->title);
    err = write(file_dev, textbuff, &size); // track title
    if (err != ERR_SUCCESS) {
        printf("error saving title length, %d (%02x)\n", err, err);
        return err;
    }

    size = sizeof(uint8_t);
    err  = write(file_dev, &track->tempo, &size);
    if (err != ERR_SUCCESS) {
        printf("error saving tempo, %d (%02x)\n", err, err);
        return err;
    }

    size = sizeof(uint8_t);
    err  = write(file_dev, &track->pattern_count, &size); // pattern count
    if (err != ERR_SUCCESS) {
        printf("error saving pattern count, %d (%02x)\n", err, err);
        return err;
    }

    /** ARRANGEMENT */
    err = zmt_arrangement_save(track->arrangement, file_dev);
    if (err != ERR_SUCCESS) {
        printf("error saving arrangement, %d (%02x)\n", err, err);
        return err;
    }

    /** PATTERNS */
    for (uint8_t p = 0; p < track->pattern_count; p++) {
        // printf("Writing pattern %d\n", p);
        err = zmt_pattern_save(track->patterns[p], file_dev);
        if (err != ERR_SUCCESS) {
            printf("error saving patterns, %d (%02x)\n", err, err);
            return err;
        }
    }
    // printf("File saved.\n");
    err = close(file_dev);
    return err;
}
/* load a track from rom */
zos_err_t zmt_rom_load(track_t* track, uint8_t* const start, uint16_t len) {
    len; /// unreferenced
    zos_err_t err;
    uint16_t size = 0;
    uint8_t *pos = start;
    char textbuff[TRACKER_TITLE_LEN + 1];

    // TODO: do this after we've determine the pattern count?
    zmt_track_init(track); // clear out the track, before loading data

    size = 3;
    memcpy(textbuff, pos, size);
    pos += size;
    // printf("Format: %.3s, pointer: %04x, %u\n", textbuff, pos, size);

    size = sizeof(uint8_t);
    memcpy(textbuff, pos, size);
    pos += size;
    // printf("Version: %d, pointer: %04x, %u\n", textbuff[0], pos, size);

    size = TRACKER_TITLE_LEN;
    memcpy(track->title, textbuff, size);
    pos += size;
    track->title[TRACKER_TITLE_LEN] = 0x00; // NUL
    // printf("Track: %12s (read: %d), pointer: %04x, %u\n", track->title, size, pos, size);

    size = sizeof(uint8_t);
    memcpy(&track->tempo, pos, size);
    pos += size;
    // printf("Tempo: %d, pointer: %04x, %u\n", track->tempo, pos, size);

    track->current_tempo = track->tempo;

    size = sizeof(uint8_t);
    memcpy(&track->pattern_count, pos, size);
    pos += size;
    // printf("Patterns: %d, pointer: %04x, %u\n", track->pattern_count, pos, size);

    err = zmt_arrangement_rom_load(track->arrangement, &pos);
    if (err != ERR_SUCCESS) {
        printf("error reading arrangement, %d (%02x)\n", err, err);
        return err;
    }
    // printf("Arrangements: %04x\n", pos);
    // for(uint8_t i = 0; i < NUM_ARRANGEMENTS; i++) {
    //     printf("%02x ", track->arrangement[i].pattern_index);
    // }
    // printf("\n");

    for (uint8_t p = 0; p < track->pattern_count; p++) {
        // printf("Loading pattern %d, %04x\n", p, pos);
        err = zmt_pattern_rom_load(track->patterns[p], &pos);
        if (err != ERR_SUCCESS) {
            printf("error loading patterns, %d (%02x)\n", err, err);
            return err;
        }
    }

    // printf("File loaded.\n\n");
    // err = close(file_dev);
    // return ERR_NOT_IMPLEMENTED; // err;
    return ERR_SUCCESS;
}


/* advanced to the prev pattern */
uint8_t zmt_pattern_prev(track_t* track)
{
    if (current_pattern > 0)
        current_pattern--;
    else {
        current_pattern = track->pattern_count - 1;
    }
    return current_pattern;
}
/* recede to the next pattern */
uint8_t zmt_pattern_next(track_t* track)
{
    current_pattern++;
    if (current_pattern >= track->pattern_count)
        current_pattern = 0;
    return current_pattern;
}
/* set the current pattern */
uint8_t zmt_pattern_set(track_t* track, uint8_t index)
{
    current_pattern = index;
    if (current_pattern > track->pattern_count)
        current_pattern = 0;
    return current_pattern;
}

/* Initialize a step, zeroes out everything*/
void zmt_step_init(step_t* step, uint8_t index)
{
    if(step == NULL) return;
    (void*) index; // unreferenced
    step->note     = NOTE_OUT_OF_RANGE;
    step->waveform = WAVEFORM_OUT_OF_RANGE;
    step->fx1      = FX_OUT_OF_RANGE;
    step->fx2      = FX_OUT_OF_RANGE;

    // we don't talk about bruno
    step->fx1_attr = 0x00;
    step->fx2_attr = 0x00;
}

/* Initialize new voice, zeroes out everything */
void zmt_voice_init(voice_t* voice, uint8_t index)
{
    if(voice == NULL) return;
    uint8_t i;
    voice->index = index;
    for (i = 0; i < STEPS_PER_PATTERN; i++) {
        step_t* step = &voice->steps[i];
        zmt_step_init(step, i);
    }
}

/* Initialize a new pattern, zeroes out everything */
void zmt_pattern_init(pattern_t* pattern)
{
    if(pattern == NULL) return;
    uint8_t i;
    for (i = 0; i < NUM_VOICES; i++) {
        voice_t* voice = &pattern->voices[i];
        zmt_voice_init(voice, i);
    }
}

void zmt_arrangement_init(arrangement_t arrangement[NUM_ARRANGEMENTS])
{
    uint8_t i = 0;
    for (i = 0; i < NUM_ARRANGEMENTS; i++) {
        arrangement_t* a = &arrangement[i];
        a->pattern_index = ARRANGEMENT_OUT_OF_RANGE;
        // if(i % 3 == 0) {
        //   a->fx = i;
        // } else {
        a->fx = FX_OUT_OF_RANGE;
        // }
    }
    arrangement[0].pattern_index = 0;
}

/* Initialize a new track, zeroes out everything */
void zmt_track_init(track_t* track)
{
    memcpy(track->title, "New Track", 9);
    track->pattern_count = 1;
    track->tempo         = 32;
    track->current_tempo = track->tempo;
    current_pattern      = 0;
    zmt_arrangement_init(track->arrangement);
    zmt_pattern_init(track->patterns[current_pattern]);
}



/* Reset a pattern, zeroing out playback values */
void zmt_pattern_reset(pattern_t* pattern)
{
    frames    = 0;
    next_step = 0;
    for (uint8_t s = 0; s < STEPS_PER_PATTERN; s++) {
        for (uint8_t v = 0; v < NUM_VOICES; v++) {
            pattern->voices[v].steps[s].fx1_attr = 0x00;
            pattern->voices[v].steps[s].fx2_attr = 0x00;
        }
    }
}

/* Reset a track, zeroing out playback values*/
uint8_t zmt_track_reset(track_t* track, uint8_t reset_pattern)
{
    track->current_tempo = track->tempo;
    if (reset_pattern) {
        current_arrangement = 0;
        arrangement_t* a    = &track->arrangement[0];
        if (a->pattern_index != ARRANGEMENT_OUT_OF_RANGE) {
            current_pattern = a->pattern_index;
        } else {
            current_pattern = 0;
        }
        zmt_process_arrangement_fx(track, a);
    }
    frames             = 0;
    next_step          = 0;
    pattern_t* pattern = track->patterns[current_pattern];
    zmt_pattern_reset(pattern);
    return current_pattern;
}



/* sound off*/
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