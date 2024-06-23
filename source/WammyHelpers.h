#pragma once

#include "juce_core/juce_core.h"

/* Aliases. */
using MidiNote = int;
using MidiInterval = int;

namespace wammy::consts
{
    constexpr int NUM_SEMI_TONES_IN_OCTAVE = 12;
    constexpr int INITIAL_M_VOICE_HELD_DOWN_CACHE_SIZE = 12;
    constexpr int NUM_MIDI_CHANNELS = 16;
    constexpr int NUM_MIDI_NOTES = 127;
    constexpr const char* INVALID_MIDI_NOTE = "INVALID_MIDI_NOTE";

    [[maybe_unused]] constexpr size_t RING_BUFFER_SIZE = 512;
}

namespace wammy::audio_utils
{
    JUCE_NODISCARD inline MidiNote normalizeMidiNote(MidiNote midiNote)
    {
        return midiNote % wammy::consts::NUM_SEMI_TONES_IN_OCTAVE;
    }

    enum class ENote
    {
        C = 0,
        Cs,
        D,
        Ds,
        E,
        F,
        Fs,
        G,
        Gs,
        A,
        As,
        B
    };

    JUCE_NODISCARD inline std::string stringifyMidiNote(MidiNote midiNote)
    {
        switch (static_cast<ENote>(normalizeMidiNote(midiNote)))
        {
        case (ENote::C):
            return "C";
        case (ENote::Cs):
            return "C#";
        case (ENote::D):
            return "D";
        case (ENote::Ds):
            return "D#";
        case (ENote::E):
            return "E";
        case (ENote::F):
            return "F";
        case (ENote::Fs):
            return "F#";
        case (ENote::G):
            return "G";
        case (ENote::Gs):
            return "G#";
        case (ENote::A):
            return "A";
        case (ENote::As):
            return "A#";
        case (ENote::B):
            return "B";
        }

        return consts::INVALID_MIDI_NOTE;
    }
}
