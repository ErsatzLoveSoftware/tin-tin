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
    _mpeNoteOnPairs.reserve(INITIAL_M_VOICE_HELD_DOWN_CACHE_SIZE);
    updateVoiceCacheMap(
        tin_tin::defaults::triadRoot,
        tin_tin::defaults::triadType
    );
    
    // Set up MPE instrument with legacy mode by default (standard MIDI)
    // When MPE mode is enabled, we'll switch to proper MPE zones
    _mpeInstrument.enableLegacyMode(2, juce::Range<int>(1, 17));
    _mpeInstrument.addListener(this);
}

TinTinProcessor::~TinTinProcessor() noexcept
{
    _mpeInstrument.removeListener(this);
    _processedMidiBuffer.clear();
}

void TinTinProcessor::resetProcessedMidiBuffer ()
{
    juce::MidiMessage offMidiMessage{};
    for (int channelNumber = 1; channelNumber < NUM_MIDI_CHANNELS; ++channelNumber)
    {
        for (int noteNumber = 0; noteNumber < NUM_MIDI_NOTES; ++noteNumber)
        {
            offMidiMessage = juce::MidiMessage::noteOff(channelNumber, noteNumber);
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
        const MidiNote mVoiceNote = mVoiceMidiMessage.getNoteNumber();

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
    
    if (_mpeEnabled)
    {
        processMPEImpl(outMidiBuffer);
    }
    else
    {
        processImpl(outMidiBuffer);
    }
    
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
    const Triad triad = getSelectedTriad();
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

// ============================================================================
// MPE Support
// ============================================================================

void TinTinProcessor::setMPEMode(bool shouldEnableMPE)
{
    if (_mpeEnabled == shouldEnableMPE)
        return;
        
    _mpeEnabled = shouldEnableMPE;
    _mpeInstrument.releaseAllNotes();
    _mpeNoteOnPairs.clear();
    
    if (_mpeEnabled)
    {
        // Set up standard MPE lower zone (channel 1 master, channels 2-16 for notes)
        juce::MPEZone lowerZone(juce::MPEZone::Type::lower, 15, 48, 2);
        _mpeInstrument.setZoneLayout(juce::MPEZoneLayout(lowerZone));
    }
    else
    {
        // Return to legacy mode for standard MIDI
        _mpeInstrument.enableLegacyMode(2, juce::Range<int>(1, 17));
    }
}

void TinTinProcessor::processMPEImpl(juce::MidiBuffer& outMidiBuffer)
{
    // :::::::::::::: Panic :::::::::::::: 
    if (_shouldPanic)
    {
        resetProcessedMidiBuffer();
        _mpeInstrument.releaseAllNotes();
        _mpeNoteOnPairs.clear();
        _shouldPanic = false;
        return;
    }

    // Process each MIDI message through the MPE instrument
    // The MPE instrument will call our listener callbacks (noteAdded, noteReleased, etc.)
    for (const juce::MidiMessageMetadata& midiMetadata : outMidiBuffer)
    {
        const juce::MidiMessage& message = midiMetadata.getMessage();
        _currentSamplePosition = midiMetadata.samplePosition;
        
        // Let MPEInstrument handle note on/off and convert to MPE notes
        // This will trigger our listener callbacks
        _mpeInstrument.processNextMidiEvent(message);
        
        // Pass through all MPE expression messages (pitchbend, pressure, CC74/timbre)
        // for the M-voice channels - these should not be modified
        if (!_shouldMuteMVoice)
        {
            if (message.isPitchWheel() || 
                message.isChannelPressure() || 
                message.isAftertouch() ||
                (message.isController() && message.getControllerNumber() == 74)) // Timbre CC
            {
                _processedMidiBuffer.addEvent(message, midiMetadata.samplePosition);
            }
        }
    }
}

void TinTinProcessor::noteAdded(juce::MPENote newNote)
{
    _globalVoiceTick++;
    _directionTick++;
    _positionTick++;
    
    const MidiNote mVoiceNote = newNote.initialNote;
    const int mVoiceChannel = newNote.midiChannel;
    
    // Generate the M-voice note-on (pass through the original MPE note)
    if (!_shouldMuteMVoice)
    {
        auto mVoiceOnMessage = juce::MidiMessage::noteOn(
            mVoiceChannel,
            mVoiceNote,
            newNote.noteOnVelocity.asUnsignedFloat()
        );
        _processedMidiBuffer.addEvent(mVoiceOnMessage, _currentSamplePosition);
    }
    
    // Generate the T-voice
    MidiNote tVoiceNote = resolveTVoice(mVoiceNote);
    
    auto tVoiceOnMessage = juce::MidiMessage::noteOn(
        _tVoiceMidiChannel,
        tVoiceNote,
        _tVoiceVelocity
    );
    _processedMidiBuffer.addEvent(tVoiceOnMessage, _currentSamplePosition);
    
    // Store the pair for later note-off matching
    _mpeNoteOnPairs.push_back({
        newNote.noteID,
        mVoiceChannel,
        mVoiceNote,
        tVoiceNote
    });
    
    _previousMVoiceMidiNote = mVoiceNote;
}

void TinTinProcessor::noteReleased(juce::MPENote finishedNote)
{
    // Find the matching note pair and release both voices
    for (auto it = _mpeNoteOnPairs.begin(); it != _mpeNoteOnPairs.end(); ++it)
    {
        if (it->mpeNoteID == finishedNote.noteID)
        {
            // Release M-voice
            if (!_shouldMuteMVoice)
            {
                auto mVoiceOffMessage = juce::MidiMessage::noteOff(
                    it->mVoiceChannel,
                    it->mVoiceNote
                );
                _processedMidiBuffer.addEvent(mVoiceOffMessage, _currentSamplePosition);
            }
            
            // Release T-voice
            auto tVoiceOffMessage = juce::MidiMessage::noteOff(
                _tVoiceMidiChannel,
                it->tVoiceNote
            );
            _processedMidiBuffer.addEvent(tVoiceOffMessage, _currentSamplePosition);
            
            _mpeNoteOnPairs.erase(it);
            break;
        }
    }
}

void TinTinProcessor::notePressureChanged(juce::MPENote changedNote)
{
    juce::ignoreUnused(changedNote);
    // MPE pressure changes are passed through in processMPEImpl
    // T-voice doesn't receive pressure - it maintains constant velocity
}

void TinTinProcessor::notePitchbendChanged(juce::MPENote changedNote)
{
    juce::ignoreUnused(changedNote);
    // MPE pitchbend changes are passed through in processMPEImpl
    // T-voice doesn't receive pitchbend - it stays at the tintinnabuli pitch
}

void TinTinProcessor::noteTimbreChanged(juce::MPENote changedNote)
{
    juce::ignoreUnused(changedNote);
    // MPE timbre (CC74) changes are passed through in processMPEImpl
    // T-voice doesn't receive timbre modulation
}

void TinTinProcessor::noteKeyStateChanged(juce::MPENote changedNote)
{
    juce::ignoreUnused(changedNote);
    // Handle sustain pedal state changes if needed
    // For now, the MPEInstrument handles this internally
}

