"""Reader-only SDCC images must exclude RAM operations and division runtimes."""
import re
import sys
from pathlib import Path

symbols = set(re.findall(r"^\s+[0-9A-F]{8}\s+(_\w+)\s", Path(sys.argv[1]).read_text(), re.M))
required = {"_tilemap_stream_init_reader", "_tilemap_stream_rebase", "_tilemap_stream_update"}
forbidden = {"_tilemap_stream_init_ram", "_tilemap_stream_shift_buffer", "_mem_cpy",
             "__divuint", "__divsint", "__moduint", "__modsint"}
if not required <= symbols or forbidden & symbols:
    raise SystemExit(f"Missing: {sorted(required - symbols)}; unexpected: {sorted(forbidden & symbols)}")
print("stream archive: reader/rebase image excludes RAM helpers and division/modulus runtimes")
