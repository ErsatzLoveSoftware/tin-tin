#include "TinTinProcessor.h"

#include "../WammyHelpers.h"
#include "../NoteLogger.h"
#include "../containers/Traid.h"

/* Aliases */
using namespace wammy::consts;
using wammy::audio_utils::stringifyMidiNote;

TinTinProcessor::TinTinProcessor() noexcept
{
    _voiceCacheMap.reserve(NUM_SEMI_TONES_IN_OCTAVE);
    _noteOnMVoices.reserve(INITIAL_M_VOICE_HELD_DOWN_CACHE_SIZE);
    updateVoiceCacheMap(
        tin_tin::defaults::triadRoot,
        tin_tin::defaults::triadType
    );
}

TinTinProcessor::~TinTinProcessor() noexcept
{
    _processedMidiBuffer.clear();
}

// TODO: Rename.
void TinTinProcessor::cacheNoteOnPair(NoteOnPair& noteOnPair)
{
    if (noteOnPair.mVoiceMidiMessage.isNoteOn())
    {
        _globalVoiceTick++;
        _directionTick++;
        _positionTick++;
        _noteOnMVoices.emplace_back(noteOnPair);
    }
}

void TinTinProcessor::process(juce::MidiBuffer& outMidiBuffer)
{
    _processedMidiBuffer.clear();

    if (_shouldPanic) // TODO: Make this work.
    {
        for (const juce::MidiMessageMetadata& midiMetadata : outMidiBuffer)
        {
            for (int i = 0; i < 16; ++i)
            {
                juce::MidiMessage offMidiMessage = juce::MidiMessage::allNotesOff(i);
                _processedMidiBuffer.addEvent(juce::MidiMessage::allNotesOff(i), midiMetadata.samplePosition);
            }
        }

        _shouldPanic = false;
        return;
    }

    if (_bypass)
    {
        return;
    }

    // Apply T-Voice
    for (const juce::MidiMessageMetadata& midiMetadata : outMidiBuffer)
    {
        juce::MidiMessage mVoiceMidiMessage = midiMetadata.getMessage();
        MidiNote mVoiceNote = mVoiceMidiMessage.getNoteNumber();
        _processedMidiBuffer.addEvent(mVoiceMidiMessage, midiMetadata.samplePosition);

        if (mVoiceMidiMessage.isNoteOff())
        {
            
            // TODO: Turn off "T" voice!
//            std::erase(_noteOnMVoices, mVoiceNote);
        }

        _tVoiceMidiMessage = mVoiceMidiMessage;
        MidiNote tVoice = resolveTVoice(mVoiceNote);
        _tVoiceMidiMessage.setNoteNumber(tVoice);
        
        NoteOnPair noteOnPair{ midiMetadata.samplePosition, mVoiceMidiMessage, _tVoiceMidiMessage };
        cacheNoteOnPair(noteOnPair);

//        _tVoiceMidiMessage.setNoteNumber(mVoiceNote + tVoice);
        // TODO: Send set note numbers to UI component via FIFO.
        _processedMidiBuffer.addEvent(_tVoiceMidiMessage, midiMetadata.samplePosition);
    }

    outMidiBuffer.swapWith(_processedMidiBuffer);
}

void TinTinProcessor::updateVoiceCacheMap(
    std::optional<wammy::audio_utils::ENote> triadRoot,
    std::optional<ETinTinTriadType> triadType
)
{
    _triadRoot = triadRoot.has_value() ? triadRoot.value() : _triadRoot;
    _triadType = triadType.has_value() ? triadType.value() : _triadType;

    _voiceCacheMap.clear();
    const Triad triad = getSelectedTriad(); // TODO: Add to fifo buffer.
    selectedTriad = triad.stringify();
    for (MidiNote note = 0; note < NUM_SEMI_TONES_IN_OCTAVE; ++note)
    {
        _voiceCacheMap.emplace_back(
            note,
            computeInferiorVoices(note, triad),
            computeSuperiorVoices(note, triad)
        );
    }

    wammy::logger::logVoiceCache(_voiceCacheMap); // TODO: Remove.
}

IntervalPositionPair TinTinProcessor::computeSuperiorVoices(MidiNote note, const Triad& triad)
{
    note = wammy::audio_utils::normalizeMidiNote(note);

    if (note < triad.first)
    {
        return { triad.first - note, triad.second - note };
    }
    else if (note < triad.second)
    {
        return { triad.second - note, triad.third - note };
    }
    else if (note < triad.third)
    {
        return {
            triad.third - note,
            (triad.first - note) + NUM_SEMI_TONES_IN_OCTAVE
        };
    }

    // note > triad.third
    return {
        (triad.first - note) + NUM_SEMI_TONES_IN_OCTAVE,
        (triad.second - note) + NUM_SEMI_TONES_IN_OCTAVE
    };
}

IntervalPositionPair TinTinProcessor::computeInferiorVoices(MidiNote note, const Triad& triad)
{
    note = wammy::audio_utils::normalizeMidiNote(note);

    if (note <= triad.first)
    {
        return {
            (triad.third - note) - NUM_SEMI_TONES_IN_OCTAVE,
            (triad.second - note) - NUM_SEMI_TONES_IN_OCTAVE
        };
    }
    else if (note <= triad.second)
    {
        return {
            triad.first - note,
            (triad.third - note) - NUM_SEMI_TONES_IN_OCTAVE
        };
    }
    else if (note <= triad.third)
    {
        return { triad.second - note, triad.first - note };
    }

    // note > triad.third
    return { triad.third - note, triad.second - note };
}

MidiInterval TinTinProcessor::resolvedPosition(IntervalPositionPair voiceIntervalPair) const
{
    switch (tVoicePosition)
    {
    case (ETinTinPosition::FirstPosition):
        return voiceIntervalPair.firstPosition;

    case (ETinTinPosition::SecondPosition):
        return voiceIntervalPair.secondPosition;

    case (ETinTinPosition::Alternating):
        return _positionTick % 2 == 0 ?
               voiceIntervalPair.firstPosition :
               voiceIntervalPair.secondPosition;
    }

    return -1111; // Error.
}

MidiNote TinTinProcessor::resolveTVoice(MidiNote mVoice)
{
    MidiNote normalizedM_Voice = wammy::audio_utils::normalizeMidiNote(mVoice);
    for (const TinTinVoiceCache& voiceCache : _voiceCacheMap)
    {
        if (normalizedM_Voice != voiceCache.mVoice)
        {
            continue;
        }

        auto resolvePositionAndOctave = [&](
            const IntervalPositionPair& positionPair,
            const TinTinOctave& octave) -> MidiNote
        {
            MidiNote tVoice = mVoice + resolvedPosition(voiceCache.superiorVoice) +
                (NUM_SEMI_TONES_IN_OCTAVE * static_cast<int>(octave.relativeOctave));

            if (octave.isStatic)
            {
                return wammy::audio_utils::normalizeMidiNote(tVoice) +
                    (NUM_SEMI_TONES_IN_OCTAVE * static_cast<int>(octave.staticOctave));
            }

            return tVoice;
        };

        switch (tVoiceDirection)
        {
        case (ETinTinDirection::Superior):
            return resolvePositionAndOctave(voiceCache.superiorVoice, superiorOctave);

        case (ETinTinDirection::Inferior):
            return resolvePositionAndOctave(voiceCache.inferiorVoices, inferiorOctave);

        case (ETinTinDirection::Alternating):
            if (_directionTick % 2 == 0)
            {
                return resolvePositionAndOctave(voiceCache.inferiorVoices, inferiorOctave);
            }

            return resolvePositionAndOctave(voiceCache.superiorVoice, superiorOctave);
        }
    }

    return -1; // Error.
}

Triad TinTinProcessor::getSelectedTriad()
{
    switch (_triadType)
    {
    case (ETinTinTriadType::Major):
        return Triad::major(_triadRoot);

    case (ETinTinTriadType::Minor):
        return Triad::minor(_triadRoot);

    case (ETinTinTriadType::Augmented):
        return Triad::augmented(_triadRoot);

    case (ETinTinTriadType::Diminished):
        return Triad::diminished(_triadRoot);
    }

    return Triad::emptyTriad(); // Error.
}
