# Animations

Include `<zgdk/anims/anim.h>` or the umbrella `<zgdk/anims.h>` (also included by
`<zgdk.h>`). This is a byte-frame sequencer with no entity, physics, sprite,
tilemap or heap dependency. The application supplies shared immutable frame
lists and one playback state per independently animated object.

```c
#include <zgdk/anims.h>

static const uint8_t walk_frames[] = {20, 21, 22, 21};
static const anim_def walk = {walk_frames, 4, 6, ANIM_LOOP};
static anim_t animation;

void update(void) {
    anim_play(&animation, &walk);
    uint8_t tile = anim_tick(&animation);
    /* Application assigns tile to its sprite, tilemap or other consumer. */
}
```

`anim_def` stores `frames`, `count`, `rate` and `flags`. Count and rate must each
be 1–255, and the frame list must contain at least count bytes. Definitions and
lists must remain alive while referenced. All frames use the same duration.
`ANIM_LOOP` is the only interpreted flag. Frame values may be any byte, including
zero and values above 127; games commonly use tile IDs.

`anim_t` stores the definition pointer, current frame index, timer and `done`.
Both structures occupy five bytes on Z80; frame lists are additional shared
data. Zero-initialize playback states or call `anim_restart()` before first use.

| Function | Behavior |
|---|---|
| `anim_play(state, def)` | Start a different definition at frame zero. Selecting the same definition pointer preserves progress, including completion. |
| `anim_restart(state, def)` | Always reset index, timer and completion to zero, even for the same definition. |
| `anim_tick(state)` | Return the frame for this update and advance playback for the next update. |
| `anim_tile(state)` | Preview the frame the next tick will return, without advancing. Header-only `INLINE`. |

Every frame is returned by tick exactly rate times before moving to the next
frame, including the first frame after play/restart. At a frame boundary,
tick returns the outgoing frame and leaves index/timer ready for the next call;
therefore tile can preview a different frame immediately after that tick.
After each rate calls to tick, the frame index advances. A looping animation
wraps to index zero. A non-looping animation holds its last frame and sets done
after that frame's duration has elapsed; subsequent ticks preserve that state.
Selecting a completed definition again with play does not restart it. Use restart
when an event needs to replay an animation.

Passing NULL as the definition clears playback when switching from a selected
definition; restart with NULL always clears it. Tick and tile return zero when
no definition is selected. The state pointer itself must always be valid.

Timing is in calls, not wall-clock time. Calling tick once per 60 Hz game update
with rate six advances ten animation frames per second. Skipping updates pauses
playback; calling more than once advances it faster. The module does not select
animations from velocity, draw graphics, upload assets or manage sprites.

Each non-inline function occupies a separate archive member compiled for code
size. No allocation or per-frame image copy occurs. `anim_tile` has the usual
SDK INLINE restrictions on SDCC: include its definition at the call site and do
not take its address.

Run `sh tests/run-anims.sh` for ASan/UBSan checks of looping and completion,
repeated selection, restart/switching, null definitions, high-byte frame values,
single-frame animations and maximum count/rate. There is no alternative rendering
implementation or compiler flag associated with this module.
