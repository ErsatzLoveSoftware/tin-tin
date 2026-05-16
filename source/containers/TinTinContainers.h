#pragma once

#include "../WammyHelpers.h"

enum class ETinTinTVoiceOctave
{
    MinusThree = 1,
    MinusTwo,
    MinusOne,
    Zero, // 4
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine
};

struct IntervalPositionPair
{
    IntervalPositionPair(MidiInterval inFirstVoice, MidiInterval inSecondVoice) : first (inFirstVoice), 
                                                                                   second (inSecondVoice)
    {
    }

    MidiInterval first { 0 };
    MidiInterval second { 0 };
};

struct TinTinOctave
{
    // Stored as raw offset integer cast to the enum type (not a named enum member).
    // onChange computes: cast(selectedId - Zero). Reverse: itemId = storedOffset + Zero.
    // 0 = no shift; 5 = octave 5 (matching tVoiceStaticOctave default).
    ETinTinTVoiceOctave relativeOctave = static_cast<ETinTinTVoiceOctave>(0);
    ETinTinTVoiceOctave staticOctave   = static_cast<ETinTinTVoiceOctave>(5);
    bool isStatic = false;
};

struct TinTinVoiceTable
{
    TinTinVoiceTable() = delete;

    TinTinVoiceTable(
        const MidiNote inM_Voice,
        IntervalPositionPair&& inInferiorVoices,
        IntervalPositionPair&& inSuperiorVoices) :
        mVoice(inM_Voice),
        inferiorVoices(inInferiorVoices),
        superiorVoice(inSuperiorVoices)
    {
    }

    MidiNote mVoice{ 0 };
    
    IntervalPositionPair inferiorVoices{ 0, 0 };
    
    IntervalPositionPair superiorVoice{ 0, 0 };
};

/* Aliases */
using VoiceCacheMap = std::vector<TinTinVoiceTable>;
