# Release validation

Validated on 2026-08-14 with devkitARM r68, libgba 0.5.4, and the native mGBA 0.10.5 core.

## ROM

- File: `hush-below.gba`
- SHA-256: `32e4a5d6ad5ee7efd40364069d75337fbd1074db33e928d1c83d656a37fdb0b3`
- Size: 67,712 bytes, 4-byte aligned
- Header: `HUSH BELOW!` / `HSHB` / `00`
- Header complement: `0xDA`
- ELF: 32-bit little-endian ARM EABI5 executable with an executable load segment covering the entry point

## Automated checks

- Deterministic asset regeneration and validation: 9 indexed sheets, 11,264 bytes of 4bpp art
- Host simulation route: all three dives, sonar alerts, recovery, reports, and ending
- Python verification suite: 19 tests across ROM, ELF, and static-site integrity
- Native mGBA route: title, brief, play, sonar, pause, log, report, and ending
- Native evidence: 8 unique 240x160 PNG framebuffers
- Site proof: all capture and download paths resolve; screenshots render at 4x with nearest-neighbor scaling

The native harness drives actual GBA key masks through mGBA, reads the versioned debug-offset table from the built ROM, and asserts every screen transition before writing the framebuffer evidence in `site/assets/screens/`.
