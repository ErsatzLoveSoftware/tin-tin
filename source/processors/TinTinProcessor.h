// Learning Resources:
// "ARVO PÄRT AND THREE TYPES OF HIS TINTINNABULI TECHNIQUE" pg. 11:
// https://digital.library.unt.edu/ark:/67531/metadc271844/m2/1/high_res_d/thesis.pdf

#pragma once

#include <utility>

#include "../WammyHelpers.h"
#include "../containers/TinTinContainers.h"

#include "juce_audio_basics/juce_audio_basics.h"

/* Forward declares */
struct IntervalPositionPair;
class Triad;

enum class ETinTinDirection
{
    Superior = 1,
    Inferior,
    Alternating,
    FollowMVoiceDirection,
    CounterMVoiceDirection
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
};

namespace tin_tin::defaults
{
    constexpr wammy::audio_utils::ENote triadRoot = wammy::audio_utils::ENote::C;
    constexpr ETinTinTriadType triadType = ETinTinTriadType::Major;
    constexpr ETinTinDirection tVoiceDirection = ETinTinDirection::Superior;
    constexpr ETinTinPosition tVoicePosition = ETinTinPosition::FirstPosition;
    constexpr ETinTinTVoiceOctave tVoiceFollowingOctave = ETinTinTVoiceOctave::Zero;
    constexpr ETinTinTVoiceOctave tVoiceStaticOctave = ETinTinTVoiceOctave::Five;
    
    constexpr double tVoiceVelocity = 0.5;
    constexpr int tVoiceMidiChannel = 1;
}

class JUCE_API TinTinProcessor
{
public:
    TinTinProcessor() noexcept;

    ~TinTinProcessor() noexcept;

    inline void panic() { _shouldPanic = true; }

    inline void toggleBypass() { _bypass = !_bypass; }
    
    inline void toggleMuteMVoice() { _shouldMuteMVoice = !_shouldMuteMVoice; }

    inline void updateTVoiceVelocity(float velocity) { _tVoiceVelocity = velocity; }
    
    void resetProcessedMidiBuffer();

    void updateTVoiceMidiChannel(const int midiChannel)
    {
        if (midiChannel > wammy::consts::NUM_MIDI_CHANNELS)
        {
            std::cout << "TinTinProcessor: Invalid midi channel.";
            return;
        }
        
        _tVoiceMidiChannel = midiChannel;
    }

    void updateVoiceCacheMap(
        std::optional<wammy::audio_utils::ENote> triadRoot = std::nullopt,
        std::optional<ETinTinTriadType> chordType = std::nullopt);

    void process(juce::MidiBuffer& outMidiBuffer);

    JUCE_NODISCARD static IntervalPositionPair computeInferiorVoices(MidiNote note, const Triad& triad);

    JUCE_NODISCARD static IntervalPositionPair computeSuperiorVoices(MidiNote note, const Triad& triad);

    TinTinOctave inferiorOctave;
    TinTinOctave superiorOctave;

    ETinTinDirection tVoiceDirection = tin_tin::defaults::tVoiceDirection;
    ETinTinPosition tVoicePosition = tin_tin::defaults::tVoicePosition;

    // UI.
    juce::String selectedTriad{};

private:
    // Containers
    juce::MidiBuffer _processedMidiBuffer{};
    VoiceCacheMap _voiceTable{};

    // Options
    wammy::audio_utils::ENote _triadRoot = tin_tin::defaults::triadRoot;
    ETinTinTriadType _triadType = tin_tin::defaults::triadType;

    // Midi Messages
    juce::MidiMessage _tVoiceMidiMessage{};
    juce::MidiMessage _newestMidiMessage{};
    MidiNote _previousMVoiceMidiNote{ 0 };

    int _tVoiceMidiChannel = tin_tin::defaults::tVoiceMidiChannel;
    float _tVoiceVelocity = tin_tin::defaults::tVoiceVelocity;
    bool _shouldMuteMVoice = false;
    std::atomic_bool _shouldPanic = false;
    std::atomic_bool _bypass = false;

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

        int samplePosition;
        juce::MidiMessage mVoiceMidiMessage;
        juce::MidiMessage tVoiceMidiMessage;
    };

    std::vector<NoteOnPair> _noteOnMVoices{};
    MidiNote lastFollowTVoice{};
    MidiNote lastCounterTVoice{};
    
    JUCE_NODISCARD Triad getSelectedTriad();
    JUCE_NODISCARD MidiNote resolveTVoice(MidiNote mVoice);
    JUCE_NODISCARD MidiInterval resolvedPosition(IntervalPositionPair voiceIntervalPair) const;
    JUCE_NODISCARD MidiNote resolvePositionAndOctave(
        MidiNote mVoice,
        const TinTinOctave& octave,
        const IntervalPositionPair& positionPair);

    void cacheNoteOnPair(NoteOnPair& noteOnPair);

    void processImpl(juce::MidiBuffer& outMidiBuffer);
};
