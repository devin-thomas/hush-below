#!/usr/bin/env python3
"""Generate the deterministic indexed art pack for HUSH BELOW!."""

from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "assets/generated"
PREVIEW = ROOT / "assets/preview"

PALETTE = [
    (3, 8, 16), (5, 13, 26), (12, 28, 43), (25, 48, 62),
    (53, 201, 205), (144, 239, 226), (246, 232, 184), (239, 181, 45),
    (235, 70, 59), (96, 173, 112), (61, 108, 164), (117, 82, 148),
    (91, 111, 119), (237, 246, 228), (41, 101, 87), (161, 78, 53),
]
GLYPHS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$%:!-></ "

PIXEL_FONT = {
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "B": ("11110", "10001", "10001", "11110", "10001", "10001", "11110"),
    "C": ("01111", "10000", "10000", "10000", "10000", "10000", "01111"),
    "D": ("11110", "10001", "10001", "10001", "10001", "10001", "11110"),
    "E": ("11111", "10000", "10000", "11110", "10000", "10000", "11111"),
    "F": ("11111", "10000", "10000", "11110", "10000", "10000", "10000"),
    "G": ("01111", "10000", "10000", "10111", "10001", "10001", "01111"),
    "H": ("10001", "10001", "10001", "11111", "10001", "10001", "10001"),
    "I": ("11111", "00100", "00100", "00100", "00100", "00100", "11111"),
    "J": ("00111", "00010", "00010", "00010", "10010", "10010", "01100"),
    "K": ("10001", "10010", "10100", "11000", "10100", "10010", "10001"),
    "L": ("10000", "10000", "10000", "10000", "10000", "10000", "11111"),
    "M": ("10001", "11011", "10101", "10101", "10001", "10001", "10001"),
    "N": ("10001", "11001", "10101", "10011", "10001", "10001", "10001"),
    "O": ("01110", "10001", "10001", "10001", "10001", "10001", "01110"),
    "P": ("11110", "10001", "10001", "11110", "10000", "10000", "10000"),
    "Q": ("01110", "10001", "10001", "10001", "10101", "10010", "01101"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "S": ("01111", "10000", "10000", "01110", "00001", "00001", "11110"),
    "T": ("11111", "00100", "00100", "00100", "00100", "00100", "00100"),
    "U": ("10001", "10001", "10001", "10001", "10001", "10001", "01110"),
    "V": ("10001", "10001", "10001", "10001", "10001", "01010", "00100"),
    "W": ("10001", "10001", "10001", "10101", "10101", "10101", "01010"),
    "X": ("10001", "10001", "01010", "00100", "01010", "10001", "10001"),
    "Y": ("10001", "10001", "01010", "00100", "00100", "00100", "00100"),
    "Z": ("11111", "00001", "00010", "00100", "01000", "10000", "11111"),
    "0": ("01110", "10001", "10011", "10101", "11001", "10001", "01110"),
    "1": ("00100", "01100", "00100", "00100", "00100", "00100", "01110"),
    "2": ("01110", "10001", "00001", "00010", "00100", "01000", "11111"),
    "3": ("11110", "00001", "00001", "01110", "00001", "00001", "11110"),
    "4": ("00010", "00110", "01010", "10010", "11111", "00010", "00010"),
    "5": ("11111", "10000", "10000", "11110", "00001", "00001", "11110"),
    "6": ("01110", "10000", "10000", "11110", "10001", "10001", "01110"),
    "7": ("11111", "00001", "00010", "00100", "01000", "01000", "01000"),
    "8": ("01110", "10001", "10001", "01110", "10001", "10001", "01110"),
    "9": ("01110", "10001", "10001", "01111", "00001", "00001", "01110"),
    "$": ("00100", "01111", "10100", "01110", "00101", "11110", "00100"),
    "%": ("11001", "11010", "00100", "01000", "10110", "00110", "00000"),
    ":": ("00000", "00100", "00100", "00000", "00100", "00100", "00000"),
    "!": ("00100", "00100", "00100", "00100", "00100", "00000", "00100"),
    "-": ("00000", "00000", "00000", "11111", "00000", "00000", "00000"),
    ">": ("10000", "01000", "00100", "00010", "00100", "01000", "10000"),
    "<": ("00001", "00010", "00100", "01000", "00100", "00010", "00001"),
    "/": ("00001", "00010", "00010", "00100", "01000", "01000", "10000"),
    " ": ("00000",) * 7,
}


def draw_label(image: Image.Image, at: tuple[int, int], label: str, color: int, scale: int = 1) -> None:
    draw = ImageDraw.Draw(image)
    origin_x, origin_y = at
    for index, glyph in enumerate(label):
        pattern = PIXEL_FONT[glyph]
        gx = origin_x + index * 6 * scale
        for y, row in enumerate(pattern):
            for x, value in enumerate(row):
                if value == "1":
                    draw.rectangle((gx + x * scale, origin_y + y * scale,
                                    gx + (x + 1) * scale - 1, origin_y + (y + 1) * scale - 1), fill=color)


def indexed(size: tuple[int, int], fill: int = 0) -> Image.Image:
    image = Image.new("P", size, fill)
    flat = [channel for color in PALETTE for channel in color] + [0] * (768 - 48)
    image.putpalette(flat)
    image.info["transparency"] = 0
    return image


def tile_bytes(image: Image.Image) -> bytes:
    pixels = image.load()
    output = bytearray()
    for ty in range(0, image.height, 8):
        for tx in range(0, image.width, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    output.append(pixels[tx + x, ty + y] | (pixels[tx + x + 1, ty + y] << 4))
    return bytes(output)


def bgr555(color: tuple[int, int, int]) -> int:
    r, g, b = color
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def export(name: str, image: Image.Image, frame: tuple[int, int], notes: str) -> dict:
    OUT.mkdir(parents=True, exist_ok=True)
    image.save(OUT / f"{name}.png", optimize=False)
    data = tile_bytes(image)
    words = [int.from_bytes(data[i:i + 4], "little") for i in range(0, len(data), 4)]
    guard = f"HUSH_BELOW_{name.upper()}_H"
    (OUT / f"{name}.h").write_text(
        f"#ifndef {guard}\n#define {guard}\n\n"
        f"#define {name}TilesLen {len(data)}\nextern const unsigned int {name}Tiles[{len(words)}];\n"
        f"#define {name}PalLen 32\nextern const unsigned short {name}Pal[16];\n\n#endif\n",
        encoding="ascii",
    )
    rows = []
    for start in range(0, len(words), 8):
        rows.append("    " + ",".join(f"0x{word:08X}" for word in words[start:start + 8]) + ",")
    palette = ",".join(f"0x{bgr555(color):04X}" for color in PALETTE)
    (OUT / f"{name}.c").write_text(
        f'#include "{name}.h"\n\nconst unsigned int {name}Tiles[{len(words)}] __attribute__((aligned(4))) = {{\n'
        + "\n".join(rows) + f"\n}};\n\nconst unsigned short {name}Pal[16] __attribute__((aligned(4))) = {{{palette}}};\n",
        encoding="ascii",
    )
    return {"name": name, "size": list(image.size), "frame_size": list(frame),
            "frames": (image.width // frame[0]) * (image.height // frame[1]),
            "tiles": len(data) // 32, "bytes_4bpp": len(data), "notes": notes}


def cave_tiles() -> Image.Image:
    image = indexed((128, 32), 1)
    draw = ImageDraw.Draw(image)
    for tile in range(64):
        x, y = (tile % 16) * 8, (tile // 16) * 8
        kind = tile // 16
        draw.rectangle((x, y, x + 7, y + 7), fill=1 + kind)
        for n in range(5):
            px = x + ((tile * 3 + n * 5) % 8)
            py = y + ((tile * 7 + n * 3) % 8)
            draw.point((px, py), fill=2 + ((kind + n) % 3))
        if tile % 8 == 1: draw.line((x, y + 6, x + 7, y + 2), fill=12)
        if tile % 8 == 2: draw.arc((x, y, x + 7, y + 7), 20, 220, fill=14)
        if tile % 8 == 3: draw.line((x + 3, y, x + 3, y + 7), fill=15)
        if tile % 8 == 4: draw.ellipse((x + 2, y + 2, x + 5, y + 5), fill=4)
        if tile % 8 == 5: draw.line((x, y + 3, x + 7, y + 3), fill=10)
    return image


def submarine() -> Image.Image:
    image = indexed((64, 64))
    for row in range(4):
        for col in range(4):
            x, y = col * 16, row * 16
            frame = indexed((16, 16))
            draw = ImageDraw.Draw(frame)
            draw.ellipse((3, 5, 12, 11), fill=7, outline=6)
            draw.rectangle((6, 3, 10, 6), fill=10)
            draw.point((9, 4), fill=5)
            draw.line((1, 8, 4, 8), fill=8)
            draw.line((12, 8, 15, 8), fill=4 if col == 2 else 12)
            if col == 1: draw.line((2, 12, 13, 12), fill=5)
            if col == 3: draw.rectangle((5, 6, 11, 10), outline=8)
            if row == 0: frame = frame.transpose(Image.Transpose.ROTATE_90)
            elif row == 1: frame = frame.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
            elif row == 3: frame = frame.transpose(Image.Transpose.ROTATE_270)
            image.paste(frame, (x, y))
    return image


def creatures() -> Image.Image:
    image = indexed((64, 32))
    draw = ImageDraw.Draw(image)
    for row in range(2):
        for col in range(4):
            x, y = col * 16, row * 16
            draw.ellipse((x + 2, y + 4, x + 12, y + 12), fill=11 + row, outline=2)
            draw.polygon((x + 11, y + 5, x + 15, y + 2 + row * 3, x + 13, y + 8), fill=11 + row)
            draw.point((x + 4, y + 7), fill=8)
            draw.point((x + 5, y + 7), fill=6)
            draw.line((x + 3, y + 12, x + 1, y + 15), fill=4)
            if col & 1: draw.line((x + 6, y + 3, x + 9, y), fill=5)
    return image


def specimens() -> Image.Image:
    image = indexed((64, 16))
    draw = ImageDraw.Draw(image)
    shapes = (4, 9, 7, 8)
    for col, color in enumerate(shapes):
        x = col * 16
        draw.ellipse((x + 4, 4, x + 11, 11), fill=color, outline=6)
        draw.line((x + 7, 2, x + 7, 13), fill=5)
        draw.line((x + 2, 7, x + 13, 7), fill=13)
    return image


def sonar_ring() -> Image.Image:
    image = indexed((128, 32))
    draw = ImageDraw.Draw(image)
    for col, radius in enumerate((5, 9, 13, 15)):
        x = col * 32
        box = (x + 16 - radius, 16 - radius, x + 16 + radius, 16 + radius)
        draw.ellipse(box, outline=5)
        if radius > 5:
            draw.ellipse((box[0] + 1, box[1] + 1, box[2] - 1, box[3] - 1), outline=4)
        if col > 1: draw.arc((x + 2, 2, x + 30, 30), 205, 335, fill=13)
    return image


def small_fx() -> Image.Image:
    image = indexed((64, 8))
    draw = ImageDraw.Draw(image)
    for col in range(8):
        x = col * 8
        draw.ellipse((x + (col % 3), 5 - (col % 4), x + 2 + (col % 3), 7 - (col % 4)), outline=5)
    return image


def font_sheet() -> Image.Image:
    image = indexed((128, 24))
    for index, glyph in enumerate(GLYPHS):
        x, y = (index % 16) * 8, (index // 16) * 8
        draw_label(image, (x + 1, y), glyph, 6)
    return image


def wordmark() -> Image.Image:
    image = indexed((96, 32))
    draw = ImageDraw.Draw(image)
    draw_label(image, (24, 1), "HUSH", 6, 2)
    draw_label(image, (12, 17), "BELOW!", 7, 2)
    draw.line((0, 14, 18, 14), fill=4, width=2)
    draw.line((76, 14, 95, 14), fill=4, width=2)
    draw.ellipse((2, 3, 13, 26), outline=4)
    draw.arc((80, 2, 94, 29), 90, 270, fill=5, width=2)
    return image


def icons() -> Image.Image:
    image = indexed((64, 8))
    draw = ImageDraw.Draw(image)
    for col in range(8):
        x = col * 8
        if col == 0: draw.arc((x + 1, 1, x + 6, 6), 20, 340, fill=4)
        elif col == 1: draw.ellipse((x + 2, 2, x + 5, 5), fill=5)
        elif col == 2: draw.rectangle((x + 1, 2, x + 6, 5), outline=7)
        elif col == 3: draw.polygon((x + 4, 0, x + 7, 7, x, 7), fill=8)
        else: draw.line((x + 1, 6, x + 6, 1 + col % 3), fill=6 + col % 3)
    return image


def make_preview(tiles: Image.Image, sub: Image.Image, fish: Image.Image, samples: Image.Image) -> None:
    canvas = indexed((240, 160), 1)
    for y in range(0, 160, 8):
        for x in range(0, 240, 8):
            tile = ((x // 8) * 3 + (y // 8) * 5) % 64
            canvas.paste(tiles.crop(((tile % 16) * 8, (tile // 16) * 8,
                                     (tile % 16) * 8 + 8, (tile // 16) * 8 + 8)), (x, y))
    def paste_sprite(sprite: Image.Image, position: tuple[int, int]) -> None:
        mask = Image.new("L", sprite.size)
        mask.putdata([255 if value else 0 for value in sprite.get_flattened_data()])
        canvas.paste(sprite, position, mask)

    paste_sprite(sub.crop((0, 0, 16, 16)), (108, 74))
    for pos, frame in (((48, 48), 0), ((184, 96), 1), ((152, 40), 2)):
        sprite = fish.crop((frame * 16, 0, frame * 16 + 16, 16))
        paste_sprite(sprite, pos)
    for pos, frame in (((32, 120), 0), ((200, 32), 2), ((72, 72), 3)):
        sprite = samples.crop((frame * 16, 0, frame * 16 + 16, 16))
        paste_sprite(sprite, pos)
    draw = ImageDraw.Draw(canvas)
    draw.rectangle((0, 0, 239, 15), fill=0)
    draw_label(canvas, (4, 1), "O2 73 SPEC 2/3 A PING", 6)
    PREVIEW.mkdir(parents=True, exist_ok=True)
    canvas.save(PREVIEW / "hush_below_preview.png")
    canvas.resize((720, 480), Image.Resampling.NEAREST).save(PREVIEW / "hush_below_preview_3x.png")


def main() -> None:
    assets = []
    generated = {
        "cave_tiles": (cave_tiles(), (8, 8), "64 modular abyss, rock, vent, wreck, and luminous tiles."),
        "submarine": (submarine(), (16, 16), "Four directions with thrust, coast, ping, and brake silhouettes."),
        "creatures": (creatures(), (16, 16), "Four hunter shapes with two alert states."),
        "specimens": (specimens(), (16, 16), "Four recoverable luminous research specimens."),
        "sonar_ring": (sonar_ring(), (32, 32), "Four expanding sonar pulse frames."),
        "bubbles": (small_fx(), (8, 8), "Eight bubble and engine-wash particles."),
        "font": (font_sheet(), (8, 8), f"48 HUD glyphs in order: {GLYPHS}"),
        "wordmark": (wordmark(), (8, 8), "HUSH BELOW! title wordmark."),
        "icons": (icons(), (8, 8), "Oxygen, specimen, sonar, warning, and dive-log icons."),
    }
    for name, (image, frame, notes) in generated.items():
        assets.append(export(name, image, frame, notes))
    manifest = {"name": "HUSH BELOW! original GBA asset pack", "revision": 1,
                "screen": [240, 160], "bpp": 4, "palette": PALETTE,
                "transparent_index": 0, "assets": assets}
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    make_preview(generated["cave_tiles"][0], generated["submarine"][0],
                 generated["creatures"][0], generated["specimens"][0])
    print(f"Generated {len(assets)} indexed assets")


if __name__ == "__main__":
    main()
