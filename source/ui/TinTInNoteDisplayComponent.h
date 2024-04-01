#pragma once

#include "juce_gui_extra/juce_gui_extra.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"

class TinTinNoteDisplayComponent final :
    public juce::Component,
    public juce::AudioProcessorParameter::Listener,
    public juce::Timer
{
public:
    TinTinNoteDisplayComponent()
    {
        addAndMakeVisible(_selectedTriad);
        _selectedTriad.setBounds(20, 50, 300, 120);
        _selectedTriad.setColour(juce::Colour::fromRGB(255, 255, 255));
    }

    inline void setTriad(const juce::String& triad)
    {
        _selectedTriad.setText(triad);
    }
    
    void parameterValueChanged (int parameterIndex, float newValue) override { }
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override { }

    void paint (juce::Graphics& g) override
    {
        const auto localBounds = getLocalBounds();
        _selectedTriad.setBounds(localBounds);
    }

private:
    juce::DrawableText _selectedTriad{  };
    
private:
};