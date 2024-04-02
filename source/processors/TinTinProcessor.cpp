#include "TinTinProcessor.h"

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
        _noteOnMVoices.push_back(noteOnPair);
    }
}

void TinTinProcessor::processImpl(juce::MidiBuffer& outMidiBuffer)
{
    // :::::::::::::: Panic :::::::::::::: 
    if (_shouldPanic) // TODO: Make this work.
    {
        juce::MidiMessage offMidiMessage{};
        for (int channelNumber = 1; channelNumber < NUM_MIDI_CHANNELS; ++channelNumber)
        {
            for (int noteNumber = 0; noteNumber < NUM_MIDI_NOTES; ++noteNumber)
            {
                offMidiMessage = juce::MidiMessage::noteOff(
                    channelNumber,
                    noteNumber
                );

                _processedMidiBuffer.addEvent(offMidiMessage, 0);
            }
        }

        _shouldPanic = false;
        return;
    }

    // :::::::::::::: Bypass :::::::::::::: 
    if (_bypass)
    {
        return;
    }

    // :::::::::::::: Apply T-Voice ::::::::::::::
    for (const juce::MidiMessageMetadata& midiMetadata : outMidiBuffer)
    {
        juce::MidiMessage mVoiceMidiMessage = midiMetadata.getMessage();
        MidiNote mVoiceNote = mVoiceMidiMessage.getNoteNumber();
        _processedMidiBuffer.addEvent(mVoiceMidiMessage, midiMetadata.samplePosition);

        if (mVoiceMidiMessage.isNoteOff()) // Turn off voice pair.
        {
            for (const NoteOnPair& noteOnPair : _noteOnMVoices)
            {
                if (noteOnPair.mVoiceMidiMessage.getNoteNumber() == mVoiceNote)
                {
                    juce::MidiMessage mVoiceOffMessage = juce::MidiMessage::noteOff(
                        noteOnPair.mVoiceMidiMessage.getChannel(),
                        noteOnPair.mVoiceMidiMessage.getNoteNumber()
                    );

                    juce::MidiMessage tVoiceOffMessage = juce::MidiMessage::noteOff(
                        noteOnPair.tVoiceMidiMessage.getChannel(),
                        noteOnPair.tVoiceMidiMessage.getNoteNumber()
                    );

                    // TODO: Add logic to keep t voice held down if needed.
                    _processedMidiBuffer.addEvent(mVoiceOffMessage, midiMetadata.samplePosition);
                    _processedMidiBuffer.addEvent(tVoiceOffMessage, midiMetadata.samplePosition);
                }
            }
        }

        if (mVoiceMidiMessage.isNoteOn()) // Add t voice to out buffer.
        {
            _tVoiceMidiMessage = mVoiceMidiMessage;
            MidiNote tVoiceNote = resolveTVoice(mVoiceNote);
            _tVoiceMidiMessage.setNoteNumber(tVoiceNote);

            // TODO: Send note numbers to UI component via FIFO.
            _processedMidiBuffer.addEvent(_tVoiceMidiMessage, midiMetadata.samplePosition);

            NoteOnPair noteOnPair{ midiMetadata.samplePosition, mVoiceMidiMessage, _tVoiceMidiMessage };
            cacheNoteOnPair(noteOnPair);
        }
    }
}

void TinTinProcessor::process(juce::MidiBuffer& outMidiBuffer)
{
    _processedMidiBuffer.clear();
    processImpl(outMidiBuffer);

    outMidiBuffer.swapWith(_processedMidiBuffer); // Return.
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

    juce::Logger::outputDebugString("tVoicePosition is out of bounds of ETinTinPosition options.");

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

        auto resolvePositionAndOctave = [&](const TinTinOctave& octave) -> MidiNote
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
            return resolvePositionAndOctave(superiorOctave);

        case (ETinTinDirection::Inferior):
            return resolvePositionAndOctave(inferiorOctave);

        case (ETinTinDirection::Alternating):
            if (_directionTick % 2 == 0)
            {
                return resolvePositionAndOctave(inferiorOctave);
            }

            return resolvePositionAndOctave(superiorOctave);
        }
    }

    juce::Logger::outputDebugString("Out of bounds from switch options..");

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
