# Super VelocityCurve

A free, open MIDI utility for finger drummers and pad players. Shape **per-pad velocity curves** with controller-specific profiles that work across any drum instrument plugin.

## Features

- Per-pad velocity curves with multi-point editor, presets (incl. power), floor/ceiling (MIDI 0–127)
- Monotonic curve enforcement, calibration wizard with preview, per-pad + global histograms
- Per-pad retrigger guard, velocity gate (drop or clamp-to-floor), aftertouch curve editor
- Note remap table, channel filter/transform, pad groups with group curve paste
- A/B compare (Capture A / Hear A-B), copy/paste curves, unmapped-note warnings
- User profile system: save, duplicate, import/export (`.svcp` / `.xml`)
- Factory templates: GM, Blank, Launchpad 8×8, Maschine 16, SPD-SX, Yamaha FGDP + Rx
- Standalone MIDI input + output device pickers (use IAC / loopMIDI for virtual ports)
- MIDI 2.0-ready float pipeline (Auto / MIDI 1.0 / MIDI 2.0 output)
- AU, VST3, Standalone (unsigned, free distribution)

## Host setup

### Ableton Live vs other DAWs (dual-build plan)

**Current (v0.4):** one VST3 built as **Instrument** with a dummy stereo output bus so Ableton Live can load it.

**Best practice (v0.5, planned):** ship **two VST3 variants** from the same source ([JUCE forum consensus](https://forum.juce.com/t/compiling-two-plugins-one-as-synth-and-another-as-midi-effect/41298)):

| Variant | VST3 category | Audio bus | DAW |
|---------|---------------|-----------|-----|
| `Super VelocityCurve` (Instrument) | Instrument | Stereo pass-through | **Ableton Live**, FL Studio |
| `Super VelocityCurve MIDI FX` | Fx / MIDI effect | None or minimal | **Reaper**, **Bitwig**, Studio One |

- **AU:** keep `aumf` (Music Effect); Logic inserts MIDI FX in the MIDI FX slot natively.
- **CLAP** (future): proper Note FX category for Bitwig.
- VST3 has no portable “MIDI FX” category; `IS_MIDI_EFFECT` behaves inconsistently across hosts.

### Ableton Live (macOS / Windows)

Ableton does **not** support third-party plugins in the built-in MIDI FX slot. Use the **VST3 Instrument** build:

1. Install `Super VelocityCurve.vst3` to the system VST3 folder
2. Preferences → Plug-Ins → enable VST3 system folder → **Rescan**
3. Look under **Instruments** (not Audio Effects, not MIDI FX)
4. Create a MIDI track and load Super VelocityCurve as the instrument
5. Create a second MIDI track with your drum plugin
6. On track 2: **MIDI From** → track 1 → **Super VelocityCurve** → Monitor **In**

### Logic Pro

Use the **AU** build on a software instrument track or as a MIDI processor depending on your routing workflow.

## Build

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

## Unsigned distribution

No paid developer accounts required. macOS users may need to allow the plugin in System Settings or run `xattr -cr` on the bundle.

## License

MIT
