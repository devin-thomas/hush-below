#!/usr/bin/env python3
"""Drive HUSH BELOW! through native mGBA and publish isolated screenshots."""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MGBA = Path(
    os.environ.get("MGBA")
    or shutil.which("mGBA")
    or "/Applications/mGBA.app/Contents/MacOS/mGBA"
)
NAMES = (
    "01-title.png", "02-dive-brief.png", "03-silent-water.png",
    "04-sonar-ping.png", "05-pause.png", "06-dive-log.png",
    "07-dive-report.png", "08-ending.png",
)

LOADER = r'''
on run argv
    set scriptPath to item 1 of argv
    set targetPid to (item 2 of argv) as integer
    tell application "System Events"
        repeat 200 times
            set matches to every application process whose unix id is targetPid
            if (count of matches) is 1 then exit repeat
            delay 0.1
        end repeat
        if (count of matches) is not 1 then error "mGBA process did not register"
        set targetProcess to item 1 of matches
        tell targetProcess
            set frontmost to true
            repeat 200 times
                if exists menu item "Scripting..." of menu 1 of menu bar item "Tools" of menu bar 1 then exit repeat
                delay 0.1
            end repeat
            click menu item "Scripting..." of menu 1 of menu bar item "Tools" of menu bar 1
            repeat 200 times
                if exists window "Scripting" then exit repeat
                delay 0.1
            end repeat
            perform action "AXRaise" of window "Scripting"
            click menu item "Load script..." of menu 1 of menu bar item "File" of menu bar 1
            set panelReady to false
            repeat 100 times
                if exists sheet 1 of window "Scripting" then
                    set panelReady to true
                    exit repeat
                end if
                if exists window "Select script to load" then
                    set panelReady to true
                    exit repeat
                end if
                delay 0.1
            end repeat
            if not panelReady then error "script chooser did not open"
        end tell
        keystroke "g" using {command down, shift down}
        delay 0.3
        keystroke scriptPath
        key code 36
        delay 0.5
        key code 36
    end tell
end run
'''

def symbol_address(elf: Path, name: str) -> int:
    result = subprocess.run(
        ["/opt/devkitpro/devkitARM/bin/arm-none-eabi-nm", "-an", str(elf)],
        check=True, capture_output=True, text=True,
    )
    matches = re.findall(rf"^([0-9a-fA-F]+)\s+[A-Za-z]\s+{re.escape(name)}$",
                         result.stdout, re.MULTILINE)
    if len(matches) != 1:
        raise RuntimeError(f"expected one {name} symbol, found {len(matches)}")
    return int(matches[0], 16)

def validate_png(path: Path) -> str:
    data = path.read_bytes()
    if not data.startswith(b"\x89PNG\r\n\x1a\n") or len(data) < 33:
        raise RuntimeError(f"invalid PNG: {path.name}")
    width, height = struct.unpack_from(">II", data, 16)
    if (width, height) != (240, 160):
        raise RuntimeError(f"wrong dimensions for {path.name}: {width}x{height}")
    if len(data) < 1000:
        raise RuntimeError(f"suspiciously small screenshot: {path.name}")
    return hashlib.sha256(data).hexdigest()

def run(args: argparse.Namespace) -> None:
    existing = subprocess.run(["pgrep", "-x", "mGBA"], check=False, capture_output=True)
    if existing.returncode == 0:
        raise RuntimeError("another mGBA process is running; close it before the isolated smoke test")
    state = symbol_address(args.elf, "hb_state")
    offsets = symbol_address(args.elf, "hb_debug_offsets")
    with tempfile.TemporaryDirectory(prefix="hush-below-mgba-") as temporary:
        root = Path(temporary)
        screens = root / "screens"
        config = root / "config"
        saves = root / "saves"
        states = root / "states"
        for directory in (screens, config, saves, states): directory.mkdir()
        result_path = root / "result.txt"
        env = os.environ.copy()
        env.update({
            "XDG_CONFIG_HOME": str(config),
            "HUSH_BELOW_STATE": f"{state:08X}",
            "HUSH_BELOW_OFFSETS": f"{offsets:08X}",
            "HUSH_BELOW_OUTPUT": str(screens),
            "HUSH_BELOW_RESULT": str(result_path),
        })
        command = [str(args.mgba), "-2", "-C", "mute=1", "-C", "pauseOnFocusLost=0",
                   "-C", "autoload=0", "-C", "autosave=0",
                   "-C", f"savegamePath={saves}", "-C", f"savestatePath={states}",
                   str(args.rom)]
        process = subprocess.Popen(command, env=env, stdout=subprocess.DEVNULL,
                                   stderr=subprocess.DEVNULL, start_new_session=True)
        try:
            loaded = subprocess.run(["osascript", "-", str(args.lua), str(process.pid)],
                                    input=LOADER, text=True, capture_output=True, timeout=45)
            if loaded.returncode:
                raise RuntimeError(loaded.stderr.strip() or "script loader failed")
            deadline = time.monotonic() + args.timeout
            while time.monotonic() < deadline and not result_path.exists():
                if process.poll() is not None: raise RuntimeError("mGBA exited early")
                time.sleep(0.1)
            if not result_path.exists(): raise RuntimeError("mGBA smoke timed out")
            result = result_path.read_text()
            if result != "PASS\n": raise RuntimeError(result.rstrip())
            actual = sorted(path.name for path in screens.glob("*.png"))
            if actual != sorted(NAMES): raise RuntimeError(f"screenshot set mismatch: {actual}")
            hashes = [validate_png(screens / name) for name in NAMES]
            if len(set(hashes)) != len(hashes): raise RuntimeError("duplicate screenshots")
            args.output.mkdir(parents=True, exist_ok=True)
            for name in NAMES:
                temporary_copy = args.output / f".{name}.tmp"
                shutil.copyfile(screens / name, temporary_copy)
                os.replace(temporary_copy, args.output / name)
            (args.output / "mgba-smoke-result.txt").write_text("PASS\n", encoding="ascii")
        finally:
            process.terminate()
            try: process.wait(timeout=5)
            except subprocess.TimeoutExpired: process.kill()

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mgba", type=Path, default=DEFAULT_MGBA)
    parser.add_argument("--rom", type=Path, default=ROOT / "hush-below.gba")
    parser.add_argument("--elf", type=Path, default=ROOT / "hush-below.elf")
    parser.add_argument("--lua", type=Path, default=ROOT / "tests/mgba_smoke.lua")
    parser.add_argument("--output", type=Path, default=ROOT / "validation/native")
    parser.add_argument("--timeout", type=float, default=90)
    args = parser.parse_args()
    try:
        run(args)
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        print(f"FAIL mGBA: {error}")
        return 1
    print(f"PASS mGBA: {len(NAMES)} unique native 240x160 captures")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
