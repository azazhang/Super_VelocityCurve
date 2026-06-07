# Super VelocityCurve

A free, open MIDI effect plugin for finger drummers and pad players. Shape **per-pad velocity curves** with controller-specific profiles that work across any drum instrument plugin.

## Features (v0.1)

- Per-pad velocity curves with visual editor and live hit feedback
- Factory controller profiles: GM Standard, Launchpad, Maschine, Roland SPD-SX, Yamaha FGDP
- MIDI 2.0-ready internal velocity pipeline (normalized float; Auto / MIDI 1.0 / MIDI 2.0 output modes)
- Retrigger guard for pad bounce
- AU, VST3, and Standalone builds
- Profile export/import (`.xml` via ValueTree)

## Build (macOS)

Requirements: Xcode, CMake 3.22+, Ninja (optional)

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Plugins install to:

- `~/Library/Audio/Plug-Ins/Components/Super VelocityCurve.component`
- `~/Library/Audio/Plug-Ins/VST3/Super VelocityCurve.vst3`

## Unsigned distribution (no paid developer accounts)

This project is intended as a **free musician tool**. Builds are **unsigned**:

- **macOS:** After first install, you may need **System Settings → Privacy & Security → Open Anyway**, or run `xattr -cr` on the plugin bundle. Local development builds from your own machine are generally trusted.
- **Windows:** Download the CI artifact zip and copy the `.vst3` folder to `C:\Program Files\Common Files\VST3\`. SmartScreen may warn on unsigned binaries.

## Usage in a DAW

Insert **Super VelocityCurve** as a **MIDI FX** on a MIDI track **before** your drum plugin. Select your controller profile, pick a pad, and drag curve control points. Play to see input → output velocity in the footer.

## License

MIT — free to use, modify, and share.
