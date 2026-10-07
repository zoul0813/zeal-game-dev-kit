# ZGDK: Sound

ZGDK provides three separate sound "engines"

* Simple 4-voice sound manipulation, via [zgdk/sound/sounds.h](../include/zgdk/sound/sounds.h)
* Basic music playback from [Piano](https://github.com/zoul0813/zeal-piano), via [zgdk/sound/music.h](../include/zgdk/sound/music.h)
* Advanced tracked player from [Zeal Music Tracker](https://github.com/zoul0813/zeal-music-tracker), via [zgdk/tracker.h](../include/zgdk/sound/tracker.h)

Tracker steps are evenly spaced by `current_tempo / 4` video frames (integer
rounding); tempos below 4 pause advancement. Pattern indices are zero-based and
must be below `pattern_count` (1–16), with allocated pattern storage. Invalid
playback data is not dereferenced. Arrangement playback wraps after its 64th
entry or an empty entry; jump effects select the destination arrangement's
pattern. Step jumps retain the current frame cadence.
