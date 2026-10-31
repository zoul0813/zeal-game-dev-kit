# ZGDK: Sound

ZGDK provides four separate sound "engines"

* Frame-driven sweep effects with priorities, via [zgdk/sound/sfx.h](../include/zgdk/sound/sfx.h)
* Simple 4-voice sound manipulation, via [zgdk/sound/sounds.h](../include/zgdk/sound/sounds.h)
* Basic music playback from [Piano](https://github.com/zoul0813/zeal-piano), via [zgdk/sound/music.h](../include/zgdk/sound/music.h)
* Advanced tracked player from [Zeal Music Tracker](https://github.com/zoul0813/zeal-music-tracker), via [zgdk/sound/tracker.h](../include/zgdk/sound/tracker.h)

For SFX/tracker-only programs, call `sound_init_hardware()` before `zmt_reset(volume)` and `sfx_init(VOICE3, volume)`. This resets the hardware without linking the simple tone-player state or helpers. Call `sound_deinit()` on exit.

Call `sound_init()` before using the simple tone player; it retains its previous behavior of initializing all four `Sound` records and resetting hardware. If also using the tracker, call it before `zmt_reset(volume)`. Music and sound effects share the same four voices, so coordinate their use to avoid interrupting one another.

The `voice` arguments to `sound_get`, `sound_set`, and `sound_play` are indices 0–3, not the SDK's `VOICE0`–`VOICE3` bitmasks. Use `sound_set` to choose the waveform for subsequent playback. Combine `WAV_SQUARE` with a `DUTY_CYCLE_*` value to select square-wave duty; the default is 50%.

`sound_play` durations are measured in calls to `sound_loop()`. A duration of zero plays continuously until stopped.

Embed tracker music with `embed_assets(target_name FILES assets/song.zmt)`, or use `embed_asset(target_name assets/song.zmt)` for a single file. The project template automatically includes `.zmt` files from the assets directory.

## Sweep effects

`SFX_SEG(wave, from_hz, to_hz, frames)` precomputes both frequency dividers in a
six-byte SDCC record. End the array with `SFX_END`; nonterminal segments last
1–255 ticks. Frequencies must be integer values from 0 through 22045 Hz, keeping
dividers below 32768 for signed 16.16 ramps. The caller owns the array for the
whole playback; constant effect tables need no copy or allocation.

```c
#include <zgdk/sound.h>
static const sfx_seg jump[] = {SFX_SEG(WAV_SQUARE, 330, 700, 8), SFX_END};
/* Startup: sound_init_hardware(); zmt_reset(VOL_75); */
/* Reserve VOICE3 in the music, then: sfx_init(VOICE3, VOL_75); */
/* On jump: sfx_play_prio(jump, 1); */
/* Once per frame: sfx_tick(); */
```

One effect plays at a time on the selected voice mask. `sfx_play()` always
replaces it; NULL or an empty array stops playback and mutes the voice.
New and replacement effects stay muted until the next `sfx_tick()` programs
their first segment. Calling `sfx_init()` again also mutes the previous voice.
`sfx_play_prio()` starts only when idle or when the new priority is at least
the active priority. Empty/NULL priority requests return zero without
interrupting playback. `sfx_active()` reports whether an effect is running.

Reserve the selected voice in music: the sequencer does not arbitrate voices
with the tracker or simple tone player. `zmt_sound_off()` mutes every voice;
call `sfx_restore()` afterward to restore the active effect's volume. Hardware
writes preserve peripheral mapping, and effect completion mutes the voice
explicitly because a zero divider alone does not silence noise.

`tests/run-sfx.sh` checks exact divider output, priority/replacement behavior,
empty requests, transitions, muting and paging under ASan/UBSan. Its binary
links only hardware lifecycle and SFX, proving no tone-player dependency.
