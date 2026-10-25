# ZGDK: Sound

ZGDK provides three separate sound "engines"

* Simple 4-voice sound manipulation, via [zgdk/sound/sounds.h](../include/zgdk/sound/sounds.h)
* Basic music playback from [Piano](https://github.com/zoul0813/zeal-piano), via [zgdk/sound/music.h](../include/zgdk/sound/music.h)
* Advanced tracked player from [Zeal Music Tracker](https://github.com/zoul0813/zeal-music-tracker), via [zgdk/sound/tracker.h](../include/zgdk/sound/tracker.h)

Call `sound_init()` before playing sounds or configuring sound settings. If also using the tracker, call it before `zmt_reset(volume)`. Music and sound effects share the same four voices, so coordinate their use to avoid interrupting one another.

The `voice` arguments to `sound_get`, `sound_set`, and `sound_play` are indices 0–3, not the SDK's `VOICE0`–`VOICE3` bitmasks. Use `sound_set` to choose the waveform for subsequent playback. Combine `WAV_SQUARE` with a `DUTY_CYCLE_*` value to select square-wave duty; the default is 50%.

`sound_play` durations are measured in calls to `sound_loop()`. A duration of zero plays continuously until stopped.

Embed tracker music with `embed_assets(target_name FILES assets/song.zmt)`, or use `embed_asset(target_name assets/song.zmt)` for a single file. The project template automatically includes `.zmt` files from the assets directory.
