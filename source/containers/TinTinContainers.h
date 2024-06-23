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
    ETinTinTVoiceOctave relativeOctave = ETinTinTVoiceOctave::Zero;
    ETinTinTVoiceOctave staticOctave = ETinTinTVoiceOctave::Zero;
    bool isStatic = false;
};

struct TinTinVoiceTable
{
    TinTinVoiceTable() = delete;

    TinTinVoiceTable(
        MidiNote inM_Voice,
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
