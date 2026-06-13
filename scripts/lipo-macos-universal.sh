#!/usr/bin/env bash
# Merge arm64 + x86_64 macOS plugin artefacts into universal binaries.
# Usage: lipo-macos-universal.sh <arm64-dist-dir> <x86_64-dist-dir> <output-dist-dir>
set -euo pipefail

if [[ $# -ne 3 ]]; then
  echo "Usage: $0 <arm64-dist> <x86_64-dist> <out-dist>" >&2
  exit 1
fi

ARM64_DIST="$(cd "$1" && pwd)"
X64_DIST="$(cd "$2" && pwd)"
OUT_DIST="$3"
mkdir -p "$OUT_DIST"

lipo_pair() {
  local rel="$1"
  local arm="$ARM64_DIST/$rel"
  local x64="$X64_DIST/$rel"
  local out="$OUT_DIST/$rel"
  if [[ ! -f "$arm" || ! -f "$x64" ]]; then
    echo "skip (missing binary): $rel" >&2
    return 0
  fi
  mkdir -p "$(dirname "$out")"
  lipo -create "$arm" "$x64" -output "$out"
  chmod +x "$out" 2>/dev/null || true
}

copy_tree() {
  local rel="$1"
  if [[ -e "$ARM64_DIST/$rel" ]]; then
    mkdir -p "$OUT_DIST/$(dirname "$rel")"
    rsync -a --delete "$ARM64_DIST/$rel/" "$OUT_DIST/$rel/" 2>/dev/null \
      || cp -R "$ARM64_DIST/$rel" "$OUT_DIST/$rel"
  fi
}

# Copy bundle trees from arm64, then replace Mach-O executables with universal slices.
for bundle in \
  "Super Velocity Curve.component" \
  "Super Velocity Curve MIDI FX.component" \
  "Super Velocity Curve.vst3" \
  "Super Velocity Curve MIDI FX.vst3" \
  "Super Velocity Curve.app" \
  "Super Velocity Curve MIDI FX.clap"
do
  [[ -e "$ARM64_DIST/$bundle" ]] || continue
  copy_tree "$bundle"

  if [[ "$bundle" == *.component ]]; then
    name="${bundle%.component}"
    lipo_pair "$bundle/Contents/MacOS/$name"
  elif [[ "$bundle" == *.vst3 ]]; then
    vst_name="${bundle%.vst3}"
    lipo_pair "$bundle/Contents/MacOS/$vst_name"
  elif [[ "$bundle" == *.app ]]; then
    lipo_pair "$bundle/Contents/MacOS/Super Velocity Curve"
  elif [[ "$bundle" == *.clap ]]; then
    lipo_pair "$bundle/Contents/MacOS/Super Velocity Curve MIDI FX"
  fi
done

# Installer assets are identical on both arches — copy from arm64 tree.
for asset in install-macos.sh "Install Super Velocity Curve.command" README.txt; do
  if [[ -e "$ARM64_DIST/$asset" ]]; then
    cp "$ARM64_DIST/$asset" "$OUT_DIST/"
  fi
done
chmod +x "$OUT_DIST/install-macos.sh" 2>/dev/null || true
chmod +x "$OUT_DIST/Install Super Velocity Curve.command" 2>/dev/null || true

echo "Universal dist written to $OUT_DIST"
