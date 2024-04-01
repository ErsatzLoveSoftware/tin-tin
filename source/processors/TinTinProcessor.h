// Learning Resources:
// "ARVO PÄRT AND THREE TYPES OF HIS TINTINNABULI TECHNIQUE" pg. 11:
// https://digital.library.unt.edu/ark:/67531/metadc271844/m2/1/high_res_d/thesis.pdf

#pragma once

#include <utility>

#include "../WammyHelpers.h"
#include "../containers/TinTinContainers.h"

#include "juce_audio_basics/juce_audio_basics.h"

/* Forward declares */
class Triad;

enum class ETinTinDirection
{
    Superior = 1,
    Inferior,
    Alternating
    // TODO: CounterM_Voice
    // TODO: FollowM_Voice
};

enum class ETinTinPosition
{
    FirstPosition = 1,
    SecondPosition,
    Alternating
};

enum class ETinTinTriadType
{
    Major = 1,
    Minor,
    Augmented,
    Diminished

    // TODO: Add a "Custom" mode.
};

namespace tin_tin::defaults
{
    constexpr wammy::audio_utils::ENote triadRoot = wammy::audio_utils::ENote::C;
    constexpr ETinTinTriadType triadType = ETinTinTriadType::Major;
    constexpr ETinTinTVoiceOctave tVoiceFollowingOctave = ETinTinTVoiceOctave::Zero;
    constexpr ETinTinTVoiceOctave tVoiceStaticOctave = ETinTinTVoiceOctave::Five;
    constexpr ETinTinDirection tVoiceDirection = ETinTinDirection::Superior;
    constexpr ETinTinPosition tVoicePosition = ETinTinPosition::FirstPosition;
}

class JUCE_API TinTinProcessor
{
public:
    TinTinProcessor() noexcept;

    ~TinTinProcessor() noexcept;

    inline void panic()
    {
        _shouldPanic = true;
    }

    inline void toggleBypass()
    {
        _bypass = !_bypass;
    }

    void updateVoiceCacheMap(
        std::optional<wammy::audio_utils::ENote> triadRoot = std::nullopt,
        std::optional<ETinTinTriadType> chordType = std::nullopt
    );

    void process(juce::MidiBuffer& outMidiBuffer);

    static IntervalPositionPair computeInferiorVoices(MidiNote note, const Triad& triad);

    static IntervalPositionPair computeSuperiorVoices(MidiNote note, const Triad& triad);

public:
    // T-Voice relativeOctave controls.
//    ETinTinTVoiceOctave inferiorOctave = tin_tin::defaults::tVoiceFollowingOctave;
//    ETinTinTVoiceOctave superiorOctave = tin_tin::defaults::tVoiceFollowingOctave;

    TinTinOctave inferiorOctave;
    TinTinOctave superiorOctave;

    ETinTinDirection tVoiceDirection = tin_tin::defaults::tVoiceDirection;
    ETinTinPosition tVoicePosition = tin_tin::defaults::tVoicePosition;

    // UI.
    juce::String selectedTriad{};

private:
    // Containers
    juce::MidiBuffer _processedMidiBuffer{};
    VoiceCacheMap _voiceCacheMap{};

    // Options.
    wammy::audio_utils::ENote _triadRoot = tin_tin::defaults::triadRoot;
    ETinTinTriadType _triadType = tin_tin::defaults::triadType;

    // Midi Messages.
    juce::MidiMessage _tVoiceMidiMessage{};
    juce::MidiMessage _newestMidiMessage{};

    bool _shouldPanic = false;
    bool _bypass = false;

    std::uint32_t _globalVoiceTick{ 0 };
    std::uint32_t _directionTick{ 0 };
    std::uint32_t _positionTick{ 0 };

    struct NoteOnPair
    {
        NoteOnPair(
            int inSamplePosition,
            juce::MidiMessage inMVoiceMidiMessage,
            juce::MidiMessage inTVoiceMidiMessage
        ) :
            samplePosition(inSamplePosition),
            mVoiceMidiMessage(std::move(inMVoiceMidiMessage)),
            tVoiceMidiMessage(std::move(inTVoiceMidiMessage))
        {
        }

        int samplePosition{0};
        juce::MidiMessage mVoiceMidiMessage;
        juce::MidiMessage tVoiceMidiMessage;
    };
    
    std::vector<NoteOnPair> _noteOnMVoices{};
    
private:
    JUCE_NODISCARD Triad getSelectedTriad();
    JUCE_NODISCARD MidiNote resolveTVoice(MidiNote mVoice);
    JUCE_NODISCARD MidiInterval resolvedPosition(IntervalPositionPair voiceIntervalPair) const;

    // TODO: Rename.
    void cacheNoteOnPair(NoteOnPair& noteOnPair);
};
