# HUSH BELOW!

HUSH BELOW! is an original sonar-stealth game for Game Boy Advance. The cave is nearly invisible until the player pings it; every revealing pulse also gives hunting creatures a target.

The ROM contains a complete three-dive campaign, Free Dive, dive reports, an ending, native PSG sound, pause/help/log screens, and deterministic original art.

## Controls

- D-pad: thrust
- A: sonar ping
- B: kill engines / brake
- Start: pause
- Select: dive log

## Build

```sh
make
make test verify size
python3 tools/run_mgba_smoke.py  # optional macOS GUI evidence lane
make site-test
```

Requires devkitARM/libgba under `/opt/devkitpro`. The finished ROM is `hush-below.gba`; the dependency-free showcase is in `site/` and includes its own downloadable copy.

`tools/mgba_headless_capture.c` is the alternate native framebuffer harness for a locally built libmgba when a GUI session is unavailable. Both native lanes derive their debug addresses from the current ELF instead of baking a build-specific address.

## Release artifacts

- Finished ROM: [`site/downloads/hush-below.gba`](site/downloads/hush-below.gba)
- Static showcase: [`site/index.html`](site/index.html)
- Asset-pack manifest: [`assets/generated/manifest.json`](assets/generated/manifest.json)
- Release evidence: [`docs/VALIDATION.md`](docs/VALIDATION.md)
