from pathlib import Path
import re
import sys
root = Path(sys.argv[1])
free = (root/'free.map').read_text()
collision = (root/'collision.map').read_text()
platformer = (root/'platformer.map').read_text()
for name in ('_body_step', '_tileflags_rows', '_tileflags_register', '_platformer_update', '_platformer_move'):
    assert name not in free, ('free body pulled in', name)
for name in ('_platformer_update', '_platformer_ladder', '_platformer_move'):
    assert name not in collision, ('collision pulled in', name)
assert '_body_init' in free
assert '_body_step' in collision and '_tileflags_rows' in collision
assert '_platformer_update' in platformer and '_platformer_move' in platformer
assert '_platformer_ladder' not in platformer, 'unused ladder code linked'
assert '_body_set_touch_mask' not in platformer, 'unused touch-mask setter linked'
print('physics archive extraction: free bodies, collision and platformer remain independent')

helpers = {"EDGES": "_body_edges", "ROW": "_body_probe_row", "COLUMN": "_body_probe_column"}
for name, symbol in helpers.items():
    linked = (root / f"helper_{name}.map").read_text()
    assert symbol in linked, (name, "helper missing")
    assert "_body_step" not in linked, (name, "pulled in collision step")
    for other in helpers.values():
        if other != symbol:
            assert other not in linked, (name, "pulled in unused helper", other)
print("physics assembly helpers: each archive member links independently")

entries = {
    "body_INIT": {"_body_init", "_body_set_pos"},
    "body_SET_POS": {"_body_set_pos"},
    "body_OVERLAP": {"_body_overlap"},
    "tileflags_REGISTER": {"_tileflags_register"},
    "tileflags_SET_OUTSIDE": {"_tileflags_set_outside"},
    "tileflags_REFRESH": {"_tileflags_refresh"},
    "tileflags_AT": {"_tileflags_at", "_tileflags_at_tile"},
    "tileflags_AT_TILE": {"_tileflags_at_tile"},
}
public = set().union(*entries.values())
for entry, expected in entries.items():
    linked = (root / f"{entry}.map").read_text()
    symbols = set(re.findall(r"\b(_(?:body|tileflags)_\w+)\b", linked))
    assert symbols & public == expected, (entry, symbols & public, expected)
    assert "_body_step" not in linked, (entry, "pulled in collision step")
    if entry.startswith("tileflags_"):
        assert not re.search(r"\b_(?:body|platformer)_\w+", linked), (entry, "pulled in physics")
    if entry.startswith("body_") or entry in ("tileflags_REGISTER", "tileflags_SET_OUTSIDE"):
        assert "_tileflags_rows" not in linked and "__TILEMAP" not in linked, (entry, "pulled in map/cache")
print("physics body and tileflags: eight entry points extract only required functions")
