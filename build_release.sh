#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/release"
PLUGIN_NAME="TinTin2"
VST3_ARTEFACT="$BUILD_DIR/${PLUGIN_NAME}_artefacts/Release/VST3/${PLUGIN_NAME}.vst3"
VST3_INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"

echo "==> Configuring (Release, VST3)..."
cmake -B "$BUILD_DIR" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG" \
    "$SCRIPT_DIR"

echo "==> Building VST3..."
cmake --build "$BUILD_DIR" --target "${PLUGIN_NAME}_VST3" --config Release -j "$(sysctl -n hw.logicalcpu)"

echo "==> Installing to $VST3_INSTALL_DIR..."
mkdir -p "$VST3_INSTALL_DIR"
rm -rf "$VST3_INSTALL_DIR/${PLUGIN_NAME}.vst3"
cp -R "$VST3_ARTEFACT" "$VST3_INSTALL_DIR/"

echo "==> Done. Plugin installed at: $VST3_INSTALL_DIR/${PLUGIN_NAME}.vst3"

'/Applications/Bitwig Studio.app/Contents/MacOS/BitwigStudio' '/Users/wammy/Documents/Bitwig Studio/Projects/TinTinPlayground/TinTinPlayground.bwproject'