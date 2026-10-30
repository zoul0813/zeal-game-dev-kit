"""Check SDCC archive extraction using the initialization-only target's map."""
import re
import sys
from pathlib import Path

symbols = set(re.findall(r"^\s+[0-9A-F]{8}\s+(_\w+)\s", Path(sys.argv[1]).read_text(), re.M))
camera_symbols = {name for name in symbols if name.startswith("_camera_")}
if camera_symbols != {"_camera_init_view"} or "_tilemap_scroll" in symbols:
    raise SystemExit(f"Unexpected camera dependencies: {sorted(camera_symbols)}")
print("camera archive: initialization-only image excludes set/follow/scroll/rebase")
