# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

TinTin2 is a JUCE MIDI effect plugin implementing Arvo Pärt's Tintinnabuli technique. It receives incoming MIDI (the "M-voice"), computes a harmonically-related "T-voice" from a selected triad, and outputs both on separate MIDI channels. It is built with CMake via the [Pamplejuce](https://github.com/sudara/pamplejuce) template.

Plugin formats built: **VST3, Standalone**.

## Build commands

Initialize submodules once after cloning:
```sh
git submodule update --init --recursive
```

Configure (CLion does this automatically; for CLI use):
```sh
cmake -B cmake-build-debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake -B cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
```

Build a specific target:
```sh
cmake --build cmake-build-debug --target TinTin2_Standalone -j 10
cmake --build cmake-build-debug --target TinTin2_VST3 -j 10
```

The Tests and Benchmarks targets are **disabled** (commented out in `CMakeLists.txt`). The CI workflow is also fully commented out.

## Code style

Formatted with `.clang-format` (JUCE style):
- 4-space indentation, no tabs
- Allman braces (`AfterClass`, `AfterFunction`, etc.)
- No column limit
- Sorted includes

Private members use `_camelCase`. Enums use `EFoo` prefix. JUCE nodiscard is used on pure query functions.

## Architecture

### Processing chain

```
DAW MIDI → PluginProcessor::processBlock()
              → TinTinProcessor::process()
                  → processImpl() iterates MidiBuffer
                      → resolveTVoice() per note-on
                          → looks up VoiceCacheMap
                          → applies direction + position + octave
                      → writes M-voice + T-voice to _processedMidiBuffer
              ← swaps processed buffer back to DAW
```

### Key classes

**`PluginProcessor`** (`source/processors/PluginProcessor.{h,cpp}`)  
Standard JUCE `AudioProcessor`. Owns the `AudioProcessorValueTreeState` (`paramTree`) and a `TinTinProcessor` instance. Creates the editor and defines all plugin parameters (triad root, triad type, direction, position, octave, velocity, MIDI channel).

**`TinTinProcessor`** (`source/processors/TinTinProcessor.{h,cpp}`)  
Core algorithm. On construction and on parameter change, builds a `VoiceCacheMap` (12 entries, one per semitone) via `updateVoiceCacheMap()`. On each `process()` call it iterates the MIDI buffer and for every note-on calls `resolveTVoice()` which looks up the cached `IntervalPositionPair` and applies the configured direction/position/octave logic. Note-on pairs are stored in `_noteOnMVoices` so the correct T-voice note-off can be issued later.

**`Triad`** (`source/containers/Traid.h`)  
Static factory: `Triad::major/minor/augmented/diminished(ENote root)`. Notes are normalized to 0–11 and sorted. Used to build the voice cache.

**`TinTinContainers.h`** (`source/containers/`)  
Core data types: `IntervalPositionPair` (two semitone intervals, first/second position), `TinTinOctave` (relative vs. static octave mode), `TinTinVoiceTable` (per-note entry with inferior and superior `IntervalPositionPair`), `VoiceCacheMap` alias.

**`WammyHelpers.h`** (`source/`)  
Type aliases (`MidiNote = int`, `MidiInterval = int`), `wammy::audio_utils::ENote` enum (C–B), `normalizeMidiNote()`, `stringifyMidiNote()`, and shared constants.

### UI

**`TinTinEditor`** (`source/ui/TinTinEditor.{h,cpp}`)  
Lays out all controls using absolute pixel positions. Wires APVTS parameters to UI via `ButtonAttachment`, `ComboBoxAttachment`, `SliderAttachment`. Direction selector change also toggles enabled state of octave sub-components.

**`TinTinOctaveComponent`** / **`OctaveComponent`** (`source/ui/TinTinOctaveComponent.h`)  
Two stacked `OctaveComponent` instances (superior voice, inferior voice). Each has a combo box for semitone offset and a "make static" toggle that switches between relative (±3 octave) and static (absolute 0–9 octave) modes.

**`TinTinComponents.h`** — Custom styled `TinTinButton` (red border) and `TinTinComboBox` (black/white).  
**`TinTInNoteDisplayComponent.h`** — Displays the active triad as text using `juce::DrawableText`.

### Tintinnabuli algorithm

`TinTinProcessor::computeSuperiorVoices` and `computeInferiorVoices` calculate, for each of the 12 chromatic notes, the two closest triad tones above (superior) or below (inferior) as semitone intervals. These are cached in `_voiceTable` at startup and on every triad change.

At runtime, `resolveTVoice()` picks superior or inferior voices based on `tVoiceDirection` (Superior / Inferior / Alternating / FollowMVoiceDirection / CounterMVoiceDirection), then `resolvedPosition()` picks first or second position, and `resolvePositionAndOctave()` applies the octave offset.

## Submodules

- `JUCE/` — JUCE framework (tracks develop branch)
- `modules/melatonin_inspector/` — UI debug inspector (included only in DEBUG builds)

Update submodules:
```sh
git submodule update --remote --merge JUCE
git submodule update --remote --merge modules/melatonin_inspector
```