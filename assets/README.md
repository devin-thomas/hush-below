# HUSH BELOW! asset pack

The pack is generated deterministically by `tools/generate_assets.py` at native GBA dimensions. Every runtime sheet is indexed, uses no more than 16 colors, aligns to 8x8 tiles, and exports both indexed PNG previews and plain C 4bpp tile arrays.

Assets cover cave tiles, the four-direction sub, hunters, specimens, sonar rings, bubbles, a 48-glyph HUD font, icons, and the title wordmark. `manifest.json` records dimensions, frame grids, palette indexes, and byte counts.

```sh
python3 tools/generate_assets.py
python3 tools/validate_assets.py
```
