from pathlib import Path
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
