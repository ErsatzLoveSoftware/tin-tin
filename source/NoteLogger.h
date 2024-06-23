//
// Created by Andrea Spiteri on 28/03/2024.
//

#pragma once

#include "WammyHelpers.h"
#include "containers/TinTinContainers.h"

using namespace  wammy::audio_utils;

namespace wammy::logger
{
    static inline void logMidiNote(MidiNote midiNote)
    {
        midiNote = wammy::audio_utils::normalizeMidiNote(midiNote);
        std::cout << stringifyMidiNote(midiNote) << '\n';
    }

    static inline void logNotePair(MidiNote mVoice, const IntervalPositionPair& tVoice)
    {
        const std::string mVoiceString = wammy::audio_utils::stringifyMidiNote(mVoice);

        const MidiNote tVoiceFirstPosition = mVoice + tVoice.first;
        const std::string firstPositionString = tVoiceFirstPosition < 0 ?
                                                stringifyMidiNote(
                                                    tVoiceFirstPosition + wammy::consts::NUM_SEMI_TONES_IN_OCTAVE) :
                                                stringifyMidiNote(tVoiceFirstPosition);

        const MidiNote tVoiceSecondPosition = mVoice + tVoice.second;
        const std::string secondPositionString = tVoiceSecondPosition < 0 ?
                                                 stringifyMidiNote(
                                                     tVoiceSecondPosition + wammy::consts::NUM_SEMI_TONES_IN_OCTAVE) :
                                                 stringifyMidiNote(tVoiceSecondPosition);

        std::cout << mVoice << "  " << mVoiceString << " -> [1st position: " <<
                  tVoice.first << " (" << firstPositionString << "), 2nd position: " <<
                  tVoice.second << " (" << secondPositionString << ")]\n";
    }

    static void logVoiceCache(const VoiceCacheMap& voiceMap)
    {
        std::cout << "Inferior Voices: \n";
        for (const TinTinVoiceTable& voicePair : voiceMap)
        {
            logNotePair(voicePair.mVoice, voicePair.inferiorVoices);
        }

        std::cout << "\nSuperior Voices: \n";
        for (const TinTinVoiceTable& voicePair : voiceMap)
        {
            logNotePair(voicePair.mVoice, voicePair.superiorVoice);
        }
    }
} // wammy::logger
