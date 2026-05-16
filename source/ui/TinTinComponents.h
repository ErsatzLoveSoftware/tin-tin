#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "../processors/PluginProcessor.h"

class TinTinButton final : public juce::TextButton
{
public:
    TinTinButton() = default;

    explicit TinTinButton(const juce::String& buttonName) : TextButton(buttonName)
    {
    }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        TextButton::paintButton(g, isMouseOverButton, isButtonDown);

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
        setColour(backgroundColourId, BLACK);
        setColour(buttonColourId, BLACK);
        setColour(outlineColourId, WHITE);

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
