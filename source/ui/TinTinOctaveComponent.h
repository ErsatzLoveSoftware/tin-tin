#pragma once

#include <utility>

#include "juce_gui_basics/juce_gui_basics.h"

#include "TinTinComponents.h"

#include "../processors/PluginProcessor.h"

namespace octave_consts
{
    constexpr int PADDING = 10;
}

class OctaveComponent final : public juce::Component
{
public:
    static constexpr int OCTAVE_COMPONENT_WIDTH = 232;
    static constexpr int OCTAVE_COMPONENT_HEIGHT = 35;

    OctaveComponent() = delete;

    explicit OctaveComponent(
        PluginProcessor& processorRef,
        TinTinOctave& octaveRef,
        juce::String&& label = "",
        juce::String&& componentLabel = "",
        int xPosition = 0,
        int yPosition = 0) : _processorRef(processorRef),
                             _octaveRef(octaveRef),
                             _uiLabel(std::move(label)),
                             _componentLabel(std::move(componentLabel))
    {
        octaveSelector.setName(_componentLabel + " " + octaveSelector.getName());
        _makeStaticToggle.setName(_componentLabel + " " + _makeStaticToggle.getName());

        setBounds(xPosition, yPosition, OCTAVE_COMPONENT_WIDTH, OCTAVE_COMPONENT_HEIGHT);
        setupComboBoxes();
        setupMakeStaticToggle();
    }

    void paint(juce::Graphics& g) override
    {
        // Text styling.
        g.setColour(juce::Colours::white);
        g.setFont(13.0f);

        constexpr int selectorPositionX = octave_consts::PADDING + 12;
        constexpr int selectorPositionY = octave_consts::PADDING + 2;
        constexpr int selectorWidth = 70;
        constexpr int selectorHeight = 18;
        const auto labelBounds = juce::Rectangle<int>(
            selectorPositionX,
            selectorPositionY,
            selectorWidth,
            selectorHeight);

        g.drawText(
            _uiLabel,
            labelBounds,
            juce::Justification::left,
            false);
    }

private:
    [[maybe_unused]] PluginProcessor& _processorRef;
    TinTinOctave& _octaveRef;
    juce::String _uiLabel;
    juce::String _componentLabel;

    // Octave selector.
    TinTinComboBox octaveSelector{ "octave selector" };
    juce::ToggleButton _makeStaticToggle{ "make static" };

private:
    void setupComboBoxes()
    {
        constexpr int selectorPositionX = octave_consts::PADDING + 60;
        constexpr int selectorPositionY = octave_consts::PADDING + 2;
        constexpr int selectorWidth = 59;
        constexpr int selectorHeight = 18;

        addAndMakeVisible(octaveSelector);
        octaveSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
        octaveSelector.onChange = [&]
        {
            if (!_octaveRef.isStatic)
            {
                _octaveRef.relativeOctave = static_cast<ETinTinTVoiceOctave> (
                    octaveSelector.getSelectedId() - static_cast<int> (ETinTinTVoiceOctave::Zero));
            }
            else
            {
                _octaveRef.staticOctave = static_cast<ETinTinTVoiceOctave> (
                    octaveSelector.getSelectedId() - static_cast<int> (ETinTinTVoiceOctave::Zero));
            }
        };

        if (_octaveRef.isStatic)
            populateStaticOctaveOptions();
        else
            populateRelativeOctaveOptions();
    }

    void populateStaticOctaveOptions()
    {
        octaveSelector.clear();
        octaveSelector.addItem("0", static_cast<int> (ETinTinTVoiceOctave::Zero));
        octaveSelector.addItem("1", static_cast<int> (ETinTinTVoiceOctave::One));
        octaveSelector.addItem("2", static_cast<int> (ETinTinTVoiceOctave::Two));
        octaveSelector.addItem("3", static_cast<int> (ETinTinTVoiceOctave::Three));
        octaveSelector.addItem("4", static_cast<int> (ETinTinTVoiceOctave::Four));
        octaveSelector.addItem("5", static_cast<int> (ETinTinTVoiceOctave::Five));
        octaveSelector.addItem("6", static_cast<int> (ETinTinTVoiceOctave::Six));
        octaveSelector.addItem("7", static_cast<int> (ETinTinTVoiceOctave::Seven));
        octaveSelector.addItem("8", static_cast<int> (ETinTinTVoiceOctave::Eight));
        octaveSelector.addItem("9", static_cast<int> (ETinTinTVoiceOctave::Nine));
        // Item IDs are enum values; stored value is a raw offset, so ID = offset + Zero
        const int itemId = static_cast<int>(_octaveRef.staticOctave) + static_cast<int>(ETinTinTVoiceOctave::Zero);
        octaveSelector.setSelectedId(itemId, juce::dontSendNotification);
    }

    void populateRelativeOctaveOptions()
    {
        octaveSelector.clear();
        octaveSelector.addItem("-3", static_cast<int> (ETinTinTVoiceOctave::MinusThree));
        octaveSelector.addItem("-2", static_cast<int> (ETinTinTVoiceOctave::MinusTwo));
        octaveSelector.addItem("-1", static_cast<int> (ETinTinTVoiceOctave::MinusOne));
        octaveSelector.addItem("0", static_cast<int> (ETinTinTVoiceOctave::Zero));
        octaveSelector.addItem("1", static_cast<int> (ETinTinTVoiceOctave::One));
        octaveSelector.addItem("2", static_cast<int> (ETinTinTVoiceOctave::Two));
        octaveSelector.addItem("3", static_cast<int> (ETinTinTVoiceOctave::Three));
        // Item IDs are enum values; stored value is a raw offset, so ID = offset + Zero
        const int itemId = static_cast<int>(_octaveRef.relativeOctave) + static_cast<int>(ETinTinTVoiceOctave::Zero);
        octaveSelector.setSelectedId(itemId, juce::dontSendNotification);
    }

    void setupMakeStaticToggle()
    {
        addAndMakeVisible(_makeStaticToggle);
        constexpr int positionX = octave_consts::PADDING + 120;
        constexpr int positionY = octave_consts::PADDING - 4;
        constexpr int width = 100;
        constexpr int height = 30;

        _makeStaticToggle.setBounds(
            positionX,
            positionY,
            width,
            height);

        // Reflect restored/initial state without triggering onClick
        _makeStaticToggle.setToggleState(_octaveRef.isStatic, juce::dontSendNotification);

        _makeStaticToggle.onClick = [&]() -> void
        {
            if (_makeStaticToggle.getToggleState())
            {
                _octaveRef.isStatic = true;
                populateStaticOctaveOptions();
                return;
            }

            _octaveRef.isStatic = false;
            populateRelativeOctaveOptions();
        };
    }
};

class TinTinOctaveComponent final : public juce::Component
{
public:
    TinTinOctaveComponent() = delete;

    ~TinTinOctaveComponent() override = default;

    explicit TinTinOctaveComponent(PluginProcessor& processor, int xPosition = 0, int yPosition = 0)
        : _processorRef(processor)
    {
        setBounds(
            xPosition,
            yPosition,
            OctaveComponent::OCTAVE_COMPONENT_WIDTH,
            OctaveComponent::OCTAVE_COMPONENT_HEIGHT * 2);

        addAndMakeVisible(_sVoiceOctaveComponent);
        addAndMakeVisible(_iVoiceOctaveComponent);

        _sVoiceOctaveComponent.setName("s voice octave");
        _iVoiceOctaveComponent.setName("i voice octave");

        const auto bounds = _sVoiceOctaveComponent.getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        _sVoiceOctaveComponent.setBounds(0, 0, width, height);
        _iVoiceOctaveComponent.setBounds(0, height, width, height);
    }

    void setInferiorVoiceEnabled(bool isEnabled)
    {
        _iVoiceOctaveComponent.setEnabled(isEnabled);
    }

    void setSuperiorVoiceEnabled(bool isEnabled)
    {
        _sVoiceOctaveComponent.setEnabled(isEnabled);
    }

private:
    PluginProcessor& _processorRef;
    OctaveComponent _sVoiceOctaveComponent{ _processorRef, _processorRef.tinTinProcessor.superiorOctave, "s voice:",
                                            "s" };
    OctaveComponent _iVoiceOctaveComponent{ _processorRef, _processorRef.tinTinProcessor.inferiorOctave, "i voice:",
                                            "i" };
};
