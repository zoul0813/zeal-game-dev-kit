# High scores

Include `<zgdk/hiscore.h>` (also included by `<zgdk.h>`). The library keeps one
table of `HISCORES_COUNT` (10) entries, ranked highest first. Each entry stores
three initials bytes and a `uint16_t` score. Initialize it with `hiscore_init()`;
provide ten descending `default_scores`, or NULL for an empty, zeroed table.
The configuration and its referenced storage must remain alive while in use.
An optional `path` loads saved scores during initialization; a failed load
leaves the defaults in place.

## Reading scores for a HUD

`hiscore_get(rank)` returns a read-only row for ranks 0–9, or NULL for an invalid
rank. Rank 0 is the game high score. `hiscore_find(initials)` returns the highest
retained score matching exactly three initials bytes, or NULL if no entry matches
or the initials pointer is NULL. The initials need not be NUL-terminated; the
pointer must address at least three bytes. Matching is case-sensitive.

```c
const highscore_t *record = hiscore_get(0);
uint16_t game_hiscore = record->score;

const highscore_t *personal = hiscore_find("ABC");
uint16_t player_hiscore = personal ? personal->score : 0;

/* Use current_score / game_hiscore or current_score / player_hiscore
 * when formatting the game's own HUD. */
```

These queries do not draw, poll input, change scores or access files. They read
the table populated by initialization, loading and score entry. The current run
isn't recorded until submitted; a HUD can display the larger of the current score
and the saved record if it should update immediately when the record is beaten.

Returned pointers refer to library-owned storage. Do not modify or free it.
`hiscore_init`, `hiscore_load` and `hiscore_add` can replace entries or shift ranks;
query again after those operations, or copy a score value for a snapshot.

Initials are the only player identifier. Different players using the same initials
share a lookup result, and scores dropped from the top ten cannot be retrieved.
The API does not maintain a separate history for every player.

## Entry, display and persistence

`hiscore_show()` and `hiscore_hide()` use the configured tile layer.
`hiscore_add(score)` retains its interactive initials-entry UI, returning the new
rank, or -1 if the score doesn't qualify. It saves automatically when a path is
configured. Equal scores are placed after existing equal entries.

`hiscore_save(path)` and `hiscore_load(path)` return `zos_err_t`. The existing
version-1 `ZHS` file format is unchanged. Loading validates the header and complete
payload before replacing the live table. Saved tables must retain descending rank
order, as produced by score entry.

Each public function occupies a separate archive member, with one shared state
member. Linking either HUD query pulls in no display, input, or filesystem code.
Initialization still links loading because it supports an optional saved path;
interactive score entry still links display, input and saving.

Run `sh tests/run-hiscore.sh` for native sanitizer checks of queries, interactive
entry and persistence. After rebuilding `zgdk.lib`, run
`python3 tests/check-hiscore-link.py` to verify archive separation and HUD query
dependencies.
