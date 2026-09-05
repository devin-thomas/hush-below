#!/usr/bin/env python3
"""Validate HUSH BELOW! generated art and export metadata."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/generated"


def main() -> None:
    manifest = json.loads((OUT / "manifest.json").read_text())
    assert manifest["bpp"] == 4
    assert len(manifest["palette"]) == 16
    total = 0
    for asset in manifest["assets"]:
        path = OUT / f"{asset['name']}.png"
        image = Image.open(path)
        assert image.mode == "P", path
        assert list(image.size) == asset["size"], path
        assert image.width % 8 == 0 and image.height % 8 == 0, path
        assert max(image.get_flattened_data()) < 16, path
        assert (OUT / f"{asset['name']}.c").is_file()
        assert (OUT / f"{asset['name']}.h").is_file()
        total += asset["bytes_4bpp"]
    assert total <= 32 * 1024
    for name in ("hush_below_preview.png", "hush_below_preview_3x.png"):
        assert (ROOT / "assets/preview" / name).is_file()
    print(f"Validated {len(manifest['assets'])} indexed sheets, {total} bytes of 4bpp art")


if __name__ == "__main__":
    main()
