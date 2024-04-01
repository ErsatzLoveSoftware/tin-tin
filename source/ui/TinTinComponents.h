#pragma once

#include <utility>

#include "juce_gui_extra/juce_gui_extra.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "../processors/PluginProcessor.h"

class TinTinButton : public juce::TextButton
{
public:
    TinTinButton() = default;

    explicit TinTinButton(const juce::String& buttonName) : juce::TextButton(buttonName)
    {
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        juce::TextButton::paintButton(g, isMouseOverButton, isButtonDown);

        g.setColour(juce::Colours::red);
        constexpr int borderThickness{ 2 };
        g.drawRect(getLocalBounds(), borderThickness);
    }
};

class TinTinComboBox final : public juce::ComboBox
{
    const juce::Colour BLACK = juce::Colour::fromRGB(0, 0, 0);
    const juce::Colour WHITE = juce::Colour::fromRGB(255, 255, 255);

public:
    explicit TinTinComboBox(const juce::String& componentName) : juce::ComboBox(componentName)
    {
        setColour(ColourIds::backgroundColourId, BLACK);
        setColour(ColourIds::buttonColourId, BLACK);
        setColour(ColourIds::outlineColourId, WHITE);

        auto popupMenu = getRootMenu();
        juce::LookAndFeel_V4 lookAndFeel;

//        isMouseOver() // TODO: research.
//        popupMenu->setLookAndFeel();
//        lookAndFeel.setColour(_popupMenu->backgroundColourId, BLACK);
//        lookAndFeel.setColour(_popupMenu->headerTextColourId, BLACK);
//        lookAndFeel.setColour(_popupMenu->highlightedBackgroundColourId, BLACK);
//        lookAndFeel.setColour(_popupMenu->textColourId, BLACK);
//        _popupMenu->setLookAndFeel(nullptr);
//        _popupMenu->setLookAndFeel(&lookAndFeel);
    }
    
private:
//    juce::PopupMenu* _popupMenu{};
};
