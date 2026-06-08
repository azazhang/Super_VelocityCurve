# Super VelocityCurve — v0.4 Backlog (completed)

Audit-derived backlog. Status after v0.4 implementation.

## Engine

| ID | Item | Status |
|----|------|--------|
| E1 | Float LUT (normalized 0–1) | **done** |
| E2 | Monotonic curve enforcement | **done** (editor + rebuildLut) |
| E3 | Retrigger thread safety | **done** (unique_lock for retrigger writes) |
| E4 | Gate clamp mode | **done** |
| E5 | Per-pad histogram | **done** |
| E6 | Power curve preset | **done** |
| E7 | Unmapped note warning | **done** (live hits status) |

## Profiles

| ID | Item | Status |
|----|------|--------|
| P1 | Launchpad 8×8 (36–99) | **done** (64 pads) |
| P2 | Maschine 16-pad group | **done** |
| P3 | FGDP + Rx pads | **done** (8 Rx pads row) |
| P4 | Blank / Custom template | **done** |
| P5 | Pad group tags | **done** |
| P6 | Factory XML resources | **done** (`Resources/FactoryProfiles/`) |
| P7 | Gate mode serialization | **done** |

## MIDI utilities UI

| ID | Item | Status |
|----|------|--------|
| M1 | Note remap table editor | **done** |
| M2 | Channel transform docs in UI | **done** (routing + remap help labels) |
| M3 | Aftertouch curve editor | **done** (separate edit mode) |

## UI / workflow

| ID | Item | Status |
|----|------|--------|
| U1 | Hit overlay per selected pad | **done** |
| U2 | Per-pad histogram + 60–90 band | **done** |
| U3 | Calibration preview curve | **done** |
| U4 | A/B compare workflow fix | **done** (Capture A + Hear A/B) |
| U5 | Group curve paste | **done** |
| U6 | Floor/ceiling MIDI labels | **done** |
| U7 | MIDI activity meters | **done** |
| U8 | Unmapped note warning | **done** |

## Standalone

| ID | Item | Status |
|----|------|--------|
| S1 | MIDI input picker | **done** |
| S2 | MIDI output picker | **done** |
| S3 | IAC/loopMIDI hint | **done** |

## CI / tests

| ID | Item | Status |
|----|------|--------|
| T1 | Monotonic user-edit test | **done** |
| T2 | Gate clamp test | **done** |
| T3 | Float LUT test | **done** (via monotonic on folded CP) |
| T4 | pluginval gate | **done** (runs; may skip unsigned) |
| T5 | Per-pad histogram test | **done** |
| T6 | Launchpad 64-pad test | **done** |

## Outstanding / next (v0.5+)

| ID | Item | Priority | Notes |
|----|------|----------|-------|
| D4 | **Dual VST3 builds** | High | Instrument for Ableton + MIDI FX for Reaper/Bitwig/Logic AU — see README |
| D5 | 16384-entry MIDI 2.0 LUT | Medium | Float pipeline ready |
| D6 | CLAP format | Medium | Bitwig/Reaper native MIDI FX |
| D7 | Phase 5 optional features | Low | Library compensation, zone routing, humanize |
| D8 | Pre-built factory `.svcp` XML files | Low | Export from code templates |
| D9 | Bundled virtual MIDI driver | Low | IAC/loopMIDI documented |
| D10 | Installers / notarization | Low | Unsigned zip OK for now |
| D11 | DAW test matrix docs | Medium | Logic, Reaper, Bitwig, Studio One |

## Intentional deviations (unchanged)

| ID | Item | Notes |
|----|------|-------|
| D1 | Ableton Instrument workaround (single VST today) | v0.5 will add second VST target |
| D2 | Unsigned distribution | Free CI artifacts |

## Validation (local)

- CMake Release build: **pass**
- ctest (8 engine tests): **pass**
- Version: **0.4.2**
