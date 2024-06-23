#include "TinTinProcessor.h"

#include "../NoteLogger.h"
#include "../containers/Traid.h"

/* Aliases. */
using namespace wammy::consts;
using wammy::audio_utils::stringifyMidiNote;

TinTinProcessor::TinTinProcessor() noexcept
{
    _voiceTable.reserve(NUM_SEMI_TONES_IN_OCTAVE);
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

void TinTinProcessor::resetProcessedMidiBuffer ()
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
            
            constexpr int sampleNumber = 0;
            _processedMidiBuffer.addEvent(offMidiMessage, sampleNumber);
        }
    }
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
    if (_shouldPanic)
    {
        resetProcessedMidiBuffer();
        _shouldPanic = false;
        return;
    }

    // :::::::::::::: Apply T-Voice ::::::::::::::
    for (const juce::MidiMessageMetadata& midiMetadata : outMidiBuffer)
    {
        juce::MidiMessage mVoiceMidiMessage = midiMetadata.getMessage();
        MidiNote mVoiceNote = mVoiceMidiMessage.getNoteNumber();

        // :::::::::::::: Note Off ::::::::::::::
        if (mVoiceMidiMessage.isNoteOff())
        {
            for (const NoteOnPair& noteOnPair : _noteOnMVoices)
            {
                if (noteOnPair.mVoiceMidiMessage.getNoteNumber() == mVoiceNote)
                { 
                    // TODO: Add logic to keep t voice held down if needed.
                    const auto mVoiceOffMessage = juce::MidiMessage::noteOff(
                        noteOnPair.mVoiceMidiMessage.getChannel(),
                        noteOnPair.mVoiceMidiMessage.getNoteNumber()
                    );

                    const auto tVoiceOffMessage = juce::MidiMessage::noteOff(
                        noteOnPair.tVoiceMidiMessage.getChannel(),
                        noteOnPair.tVoiceMidiMessage.getNoteNumber()
                    );

                    if (!_shouldMuteMVoice)
                    {
                        _processedMidiBuffer.addEvent(mVoiceOffMessage, midiMetadata.samplePosition);
                    }

                    _processedMidiBuffer.addEvent(tVoiceOffMessage, midiMetadata.samplePosition);
                }
            }
        }
        
        if (mVoiceMidiMessage.isNoteOn())
        {
            // :::::::::::::: Note On ::::::::::::::
            if (!_shouldMuteMVoice)
            {
                _processedMidiBuffer.addEvent(mVoiceMidiMessage, midiMetadata.samplePosition);
            }
            
            _tVoiceMidiMessage = mVoiceMidiMessage;
            MidiNote tVoiceNote = resolveTVoice(mVoiceNote);
            _tVoiceMidiMessage.setNoteNumber(tVoiceNote);
            _tVoiceMidiMessage.setVelocity(_tVoiceVelocity);
            _tVoiceMidiMessage.setChannel(_tVoiceMidiChannel);

            // TODO: Send note numbers to UI component via FIFO.
            _processedMidiBuffer.addEvent(_tVoiceMidiMessage, midiMetadata.samplePosition);

            NoteOnPair noteOnPair{ midiMetadata.samplePosition, mVoiceMidiMessage, _tVoiceMidiMessage };
            cacheNoteOnPair(noteOnPair);
            _previousMVoiceMidiNote = mVoiceMidiMessage.getNoteNumber();
        }
    }
}

void TinTinProcessor::process(juce::MidiBuffer& outMidiBuffer)
{
    if (_bypass)
    {
        return;
    }

    _processedMidiBuffer.clear();
    processImpl(outMidiBuffer);
    outMidiBuffer.swapWith(_processedMidiBuffer);
}

void TinTinProcessor::updateVoiceCacheMap(
    std::optional<ENote> triadRoot,
    std::optional<ETinTinTriadType> triadType
)
{
    _triadRoot = triadRoot.has_value() ? triadRoot.value() : _triadRoot;
    _triadType = triadType.has_value() ? triadType.value() : _triadType;

    _voiceTable.clear();
    const Triad triad = getSelectedTriad(); // TODO: Add to FIFO buffer.
    selectedTriad = triad.stringify();
    for (MidiNote note = 0; note < NUM_SEMI_TONES_IN_OCTAVE; ++note)
    {
        _voiceTable.emplace_back(
            note,
            computeInferiorVoices(note, triad),
            computeSuperiorVoices(note, triad)
        );
    }

#if DEBUG
    wammy::logger::logVoiceCache(_voiceTable);
#endif // DEBUG
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
        return voiceIntervalPair.first;

    case (ETinTinPosition::SecondPosition):
        return voiceIntervalPair.second;

    case (ETinTinPosition::Alternating):
        return _positionTick % 2 == 0 ?
               voiceIntervalPair.first :
               voiceIntervalPair.second;
    }

    juce::Logger::outputDebugString("tVoicePosition is out of bounds of ETinTinPosition options.");

    return -1111; // Error.
}

MidiNote TinTinProcessor::resolvePositionAndOctave(
    MidiNote mVoice,
    const TinTinOctave& octave,
    const IntervalPositionPair& positionPair
)
{
    MidiNote tVoice = mVoice + resolvedPosition(positionPair) +
                      (NUM_SEMI_TONES_IN_OCTAVE * static_cast<int>(octave.relativeOctave));

    if (octave.isStatic)
    {
        return wammy::audio_utils::normalizeMidiNote(tVoice) +
               (NUM_SEMI_TONES_IN_OCTAVE * static_cast<int>(octave.staticOctave));
    }

    return tVoice;
}

MidiNote TinTinProcessor::resolveTVoice(MidiNote mVoice)
{
    MidiNote normalizedMVoice = wammy::audio_utils::normalizeMidiNote(mVoice);
    for (const TinTinVoiceTable& voiceCache : _voiceTable)
    {
        if (normalizedMVoice != voiceCache.mVoice)
        {
            continue;
        }

        switch (tVoiceDirection)
        {
        case (ETinTinDirection::Superior):
            return resolvePositionAndOctave(mVoice, superiorOctave, voiceCache.superiorVoice);

        case (ETinTinDirection::Inferior):
            return resolvePositionAndOctave(mVoice, inferiorOctave, voiceCache.inferiorVoices);

        case (ETinTinDirection::Alternating):
            if (_directionTick % 2 == 0)
            {
                return resolvePositionAndOctave(mVoice, inferiorOctave, voiceCache.inferiorVoices);
            }

            return resolvePositionAndOctave(mVoice, superiorOctave, voiceCache.superiorVoice);

        case (ETinTinDirection::FollowMVoiceDirection):
            if (mVoice == _previousMVoiceMidiNote)
            {
                return lastFollowTVoice;
            }
            
            if ((mVoice - _previousMVoiceMidiNote) > 0)
            {
                lastFollowTVoice = resolvePositionAndOctave(mVoice, superiorOctave, voiceCache.superiorVoice); 
            }
            else
            {
                lastFollowTVoice = resolvePositionAndOctave(mVoice, inferiorOctave, voiceCache.inferiorVoices);
            }
            
            return lastFollowTVoice;

        case (ETinTinDirection::CounterMVoiceDirection):
            if (mVoice == _previousMVoiceMidiNote)
            {
                return lastCounterTVoice;
            }
            
            if ((mVoice - _previousMVoiceMidiNote) < 0)
            {
                lastCounterTVoice = resolvePositionAndOctave(mVoice, superiorOctave, voiceCache.superiorVoice);
            }
            else
            {
                lastCounterTVoice = resolvePositionAndOctave(mVoice, inferiorOctave, voiceCache.inferiorVoices);
            }

            return lastCounterTVoice;
        }
    }

    juce::Logger::outputDebugString("Out of bounds from switch options.");

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

    juce::Logger::outputDebugString("Out of bounds from triad switch options.");
    
    return Triad::emptyTriad(); // Error.
}
