#include <inttypes.h>
#include <core.h>
#include <zvb_sound.h>
#include "zgdk/sound/tracker.h"
#include "private.h"

/** Page Banking for Sound Peripheral */
const __sfr __banked __at(0xF0) mmu_page0_ro;

/* Empty arrangements terminate the song; an empty first entry previews pattern 0. */
static arrangement_t* select_arrangement(track_t* track, uint8_t index)
{
    if (index >= NUM_ARRANGEMENTS ||
        track->arrangement[index].pattern_index >= track->pattern_count)
        index = 0;
    zmt_current_arrangement = index;
    arrangement_t* arrangement = &track->arrangement[index];
    zmt_current_pattern = arrangement->pattern_index < track->pattern_count
        ? arrangement->pattern_index : 0;
    return arrangement;
}

uint8_t zmt_tick(track_t* track, uint8_t use_arrangement)
{
    if (track->pattern_count == 0 || track->pattern_count > NUM_PATTERNS)
        return 0;
    if (zmt_current_pattern >= track->pattern_count)
        zmt_current_pattern = 0;
    pattern_t* pattern = track->patterns[zmt_current_pattern];
    if (zmt_ticks >= track->current_tempo)
        zmt_ticks = 0;
    // if(ticks % ticks_PER_EIGTH == 0) { }
    if (zmt_ticks % TICKS_PER_SIXTEENTH(track->current_tempo) == 0) {
        zmt_last_step = zmt_next_step;
        zmt_play_pattern(pattern, zmt_next_step);

        zmt_next_step++;
        if (zmt_next_step >= STEPS_PER_PATTERN) {
            if (use_arrangement) {
                arrangement_t* arrangement = select_arrangement(track, zmt_current_arrangement + 1);
                zmt_process_arrangement_fx(track, arrangement);
                select_arrangement(track, zmt_current_arrangement);
            }
            zmt_next_step = 0;
        }
    }
    zmt_ticks++;

    return zmt_next_step;
}

void zmt_process_arrangement_fx(track_t* track, arrangement_t* arrangement)
{
    // NOTE: watch out for EVELYN, consecutive comparisons should account for logic
    // Check arrangement FX ...
    uint8_t value = arrangement->fx;
    if (LESSTHAN(value, 0x1F)) {
        zmt_current_arrangement = value;
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
    // pattern_t *pattern = active_pattern; // TODO: gonna have more than one pattern soon, lol

    if (RANGE(fx, FX_COUNT_0, FX_COUNT_8)) {
        if (step->fx1_attr == 0x00)
            step->fx1_attr = (fx - 0xC0);
        else
            step->fx1_attr--;
        return;
    }

    if(RANGE(fx, FX_GOTO_0, FX_GOTO_31)) {
        zmt_next_step = (fx - FX_GOTO_0) - 1;
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
            zmt_voice_volume(voice, VOL_0);
        } break;
        case FX_VOL_12_5: {
            // TODO: 8 step volume control
            zmt_voice_volume(voice, VOL_0);
        } break;
        case FX_VOL_25_0: {
            zmt_voice_volume(voice, VOL_25);
        } break;
        case FX_VOL_37_5: {
            // TODO: 8 step volume control
            zmt_voice_volume(voice, VOL_25);
        } break;
        case FX_VOL_50_0: {
            zmt_voice_volume(voice, VOL_50);
        } break;
        case FX_VOL_62_5: {
            // TODO: 8 step volume control
            zmt_voice_volume(voice, VOL_50);
        } break;
        case FX_VOL_75_0: {
            zmt_voice_volume(voice, VOL_75);
        } break;
        case FX_VOL_87_5: {
            // TODO: 8 step volume control
            zmt_voice_volume(voice, VOL_75);
        } break;
        case FX_VOL_100: {
            zmt_voice_volume(voice, VOL_100);
        } break;
    }
}

void zmt_play_step(step_t* step, sound_voice_t voice)
{
    // pattern_t *pattern = active_pattern; // TODO: gonna have more than one pattern soon, lol

    SOUND_SELECT(voice);

    // FX1 is pre-processed
    if (step->fx1 != FX_OUT_OF_RANGE)
        zmt_process_fx(step, step->fx1, voice);

    // Volume effects select every voice, so restore the current voice before
    // changing its note or waveform.
    SOUND_SELECT(voice);

    if (step->note != NOTE_OUT_OF_RANGE) {
        note_t note = NOTES[step->note];
        SOUND_DIV(note);
        zmt_voice_active(voice, step->note != NOTE_OFF);
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

    uint8_t backup_page = zvb_config_dev_idx;
    zvb_map_peripheral(ZVB_PERI_SOUND_IDX);
    zmt_play_step(step1, VOICE0);
    zmt_play_step(step2, VOICE1);
    zmt_play_step(step3, VOICE2);
    zmt_play_step(step4, VOICE3);
    zvb_map_peripheral(backup_page);
}

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

void zmt_track_init(track_t* track)
{
    mem_cpy(track->title, "New Track", 9);
    track->pattern_count = 1;
    track->tempo         = 32;
    track->current_tempo = track->tempo;
    zmt_current_pattern  = 0;
    zmt_arrangement_init(track->arrangement);
    zmt_pattern_init(track->patterns[zmt_current_pattern]);
}

void zmt_pattern_reset(pattern_t* pattern)
{
    zmt_ticks     = 0;
    zmt_next_step = 0;
    for (uint8_t s = 0; s < STEPS_PER_PATTERN; s++) {
        for (uint8_t v = 0; v < NUM_VOICES; v++) {
            pattern->voices[v].steps[s].fx1_attr = 0x00;
            pattern->voices[v].steps[s].fx2_attr = 0x00;
        }
    }
}

uint8_t zmt_track_reset(track_t* track, uint8_t reset_pattern)
{
    track->current_tempo = track->tempo;
    if (reset_pattern) {
        arrangement_t* a = select_arrangement(track, 0);
        zmt_process_arrangement_fx(track, a);
        select_arrangement(track, zmt_current_arrangement);
    }
    zmt_ticks          = 0;
    zmt_next_step      = 0;
    pattern_t* pattern = track->patterns[zmt_current_pattern];
    zmt_pattern_reset(pattern);
    return zmt_current_pattern;
}
