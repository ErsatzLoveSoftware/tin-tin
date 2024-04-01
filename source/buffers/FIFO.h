//
// Created by Andrea Spiteri on 29/03/2024.
//

#pragma once

#include "juce_core/juce_core.h"
#include "WammyHelpers.h"

// First in first out data structure.
class MidiFIFO final
{
public:
    std::unique_ptr<juce::AbstractFifo> lockFreeFifo;
    std::vector<int> midiData;
    unsigned int lastReadPosition = { 0 };

    MidiFIFO()
    {
        lockFreeFifo = std::make_unique<juce::AbstractFifo>(wammy::consts::RING_BUFFER_SIZE);
        midiData.reserve(wammy::consts::RING_BUFFER_SIZE);

        // TODO: Test what capacity is returning here.
        for (int i = 0; i < midiData.capacity(); ++i)
        {
            midiData.emplace_back(-1);
        }
    }

    void write(const int* writeData, int numToWrite)
    {
//        lockFreeFifo->write();
//        lockFreeFifo->finishedWrite(numToWrite);
    }

    void readFrom(const int* readData, int numToRead) const
    {
        int outStartIndex1;
        int outStartIndex2;
        int outBlockSize1;
        int outBlockSize2;

        lockFreeFifo->prepareToRead(numToRead, outStartIndex1, outBlockSize1, outStartIndex2, outBlockSize2);
        // TODO: 
        lockFreeFifo->finishedRead(numToRead);
    }

    ~MidiFIFO() = default;
};
