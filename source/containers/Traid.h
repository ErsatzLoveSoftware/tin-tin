#pragma once

#include <utility>

#include "../WammyHelpers.h"

/* Aliases. */
using wammy::audio_utils::ENote;
using wammy::audio_utils::normalizeMidiNote;

class Triad
{
public:
    Triad() = delete;

    // TODO: Add template function to take in any number of array size.
    JUCE_NODISCARD static Triad emptyTriad()
    {
        return { -1, -1, -1, "" };
    }

    // Returns a sorted major triad of MidiNotes given a root note.
    JUCE_NODISCARD static Triad major(ENote root)
    {
        std::array<MidiNote, 3> triad = {
            static_cast<MidiNote>(root),
            normalizeMidiNote(static_cast<MidiNote>(root) + 4),
            normalizeMidiNote(static_cast<MidiNote>(root) + 7)
        };

        juce::String triadString = Triad::makeString(triad[0], triad[1], triad[2]);
        std::sort(triad.begin(), triad.end(), std::less<>());

        return { triad[0], triad[1], triad[2], triadString };
    }

    // Returns a sorted minor triad of MidiNotes given a root note.
    JUCE_NODISCARD static Triad minor(ENote root)
    {
        std::array<MidiNote, 3> triad = {
            static_cast<MidiNote>(root),
            normalizeMidiNote(static_cast<MidiNote>(root) + 3),
            normalizeMidiNote(static_cast<MidiNote>(root) + 7)
        };

        const juce::String triadString = Triad::makeString(triad[0], triad[1], triad[2]);
        std::sort(triad.begin(), triad.end(), std::less<>());

        return { triad[0], triad[1], triad[2], triadString };
    }

    JUCE_NODISCARD static Triad augmented(ENote root)
    {
        std::array<MidiNote, 3> triad = {
            static_cast<MidiNote>(root),
            normalizeMidiNote(static_cast<MidiNote>(root) + 4),
            normalizeMidiNote(static_cast<MidiNote>(root) + 8)
        };

        const juce::String triadString = Triad::makeString(triad[0], triad[1], triad[2]);
        std::sort(triad.begin(), triad.end(), std::less<>());

        return { triad[0], triad[1], triad[2], triadString };
    }

    JUCE_NODISCARD static Triad diminished(ENote root)
    {
        std::array<MidiNote, 3> triad = {
            static_cast<MidiNote>(root),
            normalizeMidiNote(static_cast<MidiNote>(root) + 3),
            normalizeMidiNote(static_cast<MidiNote>(root) + 6)
        };

        const juce::String triadString = Triad::makeString(triad[0], triad[1], triad[2]);
        std::sort(triad.begin(), triad.end(), std::less<>());

        return { triad[0], triad[1], triad[2], triadString };
    }

    JUCE_NODISCARD inline const juce::String& stringify() const
    {
        return _triadAsString;
    }

public:
    MidiNote first;
    MidiNote second;
    MidiNote third;

private:
    juce::String _triadAsString;

private:
    Triad(
        MidiNote inFirst,
        MidiNote inSecond,
        MidiNote inThird,
        juce::String triadAsString
    ) :
        first(inFirst),
        second(inSecond),
        third(inThird),
        _triadAsString(std::move(triadAsString))
    {
    }

    JUCE_NODISCARD static juce::String makeString(MidiNote _first, MidiNote _second, MidiNote _third)
    {
        return stringifyMidiNote(_first) + " " + stringifyMidiNote(_second) + " " + stringifyMidiNote(_third);
    }
};
