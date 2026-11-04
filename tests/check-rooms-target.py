#!/usr/bin/env python3
"""Run inside ZDE after building: verify SDK room layouts and optional modules."""
import os
import re
from pathlib import Path
import shutil
import subprocess

root = Path(__file__).resolve().parent.parent
out = root / "build" / "rooms-target"
out.mkdir(parents=True, exist_ok=True)
zos = Path(os.environ["ZOS_PATH"])
zgdk = root
zvb = Path(os.environ["ZVB_SDK_PATH"])
core = Path(os.environ["COREUTILS_PATH"])
compiler = shutil.which("sdcc") or "/opt/sdcc/bin/sdcc"
linker = shutil.which("sdldz80") or "/opt/sdcc/bin/sdldz80"


def read_hex(path):
    memory = {}
    base = 0
    for line in path.read_text().splitlines():
        record = bytes.fromhex(line[1:])
        assert sum(record) & 255 == 0, "Invalid Intel HEX checksum"
        size, address, kind = record[0], int.from_bytes(record[1:3], "big"), record[3]
        if kind == 0:
            memory.update((base + address + i, byte) for i, byte in enumerate(record[4:4 + size]))
        elif kind == 4:
            base = int.from_bytes(record[4:6], "big") << 16
    return memory


def check_initializer(symbols, hex_path, sliding):
    def address(name):
        match = re.search(r"^\s*([0-9A-Fa-f]{8})\s+_" + name + r"\b", symbols, re.M)
        assert match, f"Missing probe symbol: {name}"
        return int(match[1], 16)

    memory = read_hex(hex_path)
    offset_address = address("probe_offsets")
    size, grid, screen, room, layer, tiles, capacity, loader, transition = [memory[offset_address + i] for i in range(9)]
    config_address = address("probe_config")
    config = bytes(memory[config_address + i] for i in range(size))

    def word(offset):
        return int.from_bytes(config[offset:offset + 2], "little")

    assert config[grid:grid + 2] == bytes([8, 8])
    assert (word(screen), word(screen + 2)) == ((320, 240) if sliding else (640, 480))
    assert tuple(word(room + i) for i in (0, 2, 4, 6)) == ((0, 0, 20, 15) if sliding else (0, 4, 40, 21))
    assert config[layer] == 0
    assert word(tiles) != 0 and word(loader) != 0
    assert word(capacity) == (300 if sliding else 840)
    assert bool(word(transition)) == sliding


for name, define in [("enigma-immediate", None), ("demo-slide", "PROBE_SLIDE"), ("minimap-state", "PROBE_MINIMAP")]:
    command = [compiler, "-mz80", "--codeseg", "TEXT", "--nostdlib"]
    for include in [zos / "sdk/sdcc/include", zgdk / "include", zvb / "include"]:
        command.append(f"-I{include}")
    if define:
        command.append(f"-D{define}")
    subprocess.run([*command, "-c", str(root / "tests/rooms/target.c"), "-o", str(out / f"{name}.rel")], check=True)
    command = [linker, "-n", "-b", "_HEADER=0x4000", "-i", "-m", "-w"]
    for directory, library in [(zgdk / "lib", "zgdk"),
                                (zvb / "lib", "zvb_gfx"), (core / "lib", "core"), (zos / "sdk/sdcc/lib", "z80")]:
        command.extend(["-k", str(directory), "-l", library])
    subprocess.run([*command, str(out / f"{name}.ihx"), str(zos / "sdk/sdcc/bin/zos_crt0.rel"), str(out / f"{name}.rel")], check=True)
    symbols = (out / f"{name}.map").read_text()
    assert "_minimap_render" not in symbols and "_minimap_tiles" not in symbols
    if define == "PROBE_SLIDE":
        assert "_ROOMS_TRANSITION_SLIDE" in symbols
    else:
        assert "_ROOMS_TRANSITION_SLIDE" not in symbols and "slide.c.rel" not in symbols
    if define == "PROBE_MINIMAP":
        assert "_rooms_runtime" not in symbols and "_rooms_init" not in symbols
        assert not re.search(r"\b_gfx_\w+", symbols), "state-only minimap linked graphics"
    else:
        check_initializer(symbols, out / f"{name}.ihx", define == "PROBE_SLIDE")
    print(f"PASS: Z80 {name} links from zgdk.lib with unused modules omitted", flush=True)
