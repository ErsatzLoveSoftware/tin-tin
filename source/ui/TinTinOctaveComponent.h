#pragma once

#include <utility>

#include "juce_gui_extra/juce_gui_extra.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "TinTinComponents.h"

#include "../processors/PluginProcessor.h"

namespace octave_consts
{
    constexpr int padding = 10;
}

class OctaveComponent final : public juce::Component
{
public:
    static constexpr int OCTAVE_COMPONENT_WIDTH = 232;
    static constexpr int OCTAVE_COMPONENT_HEIGHT = 35;

    explicit OctaveComponent(
        PluginProcessor& processorRef,
        TinTinOctave& octaveRef,
        juce::String label = "",
        int xPosition = 0,
        int yPosition = 0) :
        _processorRef(processorRef),
        _octaveRef(octaveRef),
        _label(std::move(label))
    {
        setBounds(xPosition, yPosition, OCTAVE_COMPONENT_WIDTH, OCTAVE_COMPONENT_HEIGHT);
        setupComboBoxes();
        setupMakeStaticToggle();
    }

    void paint(juce::Graphics& g) override
    {
        // Text styling.
        g.setColour(juce::Colours::white);
        g.setFont(13.0f);

        constexpr int selectorPositionX = octave_consts::padding + 12;
        constexpr int selectorPositionY = octave_consts::padding + 2;
        constexpr int selectorWidth = 70;
        constexpr int selectorHeight = 18;
        const auto labelBounds = juce::Rectangle<int>(
            selectorPositionX,
            selectorPositionY,
            selectorWidth,
            selectorHeight
        );

        g.drawText(
            _label,
            labelBounds,
            juce::Justification::left,
            false
        );
    }

private:
    PluginProcessor& _processorRef;
    TinTinOctave& _octaveRef; // TODO Maybe rename TinTinOctave.
    juce::String _label;

    // Octave selector.
    TinTinComboBox _relativeOctaveSelector{ "Relative Octave Selector"};
    TinTinComboBox _staticOctaveSelector{ "Static Octave Selector"};
    juce::ToggleButton _makeStaticToggle{ "Make Static"};

private:
    void setupComboBoxes()
    {
        constexpr int selectorPositionX = octave_consts::padding + 60;
        constexpr int selectorPositionY = octave_consts::padding + 2;
        constexpr int selectorWidth = 59;
        constexpr int selectorHeight = 18;

        addAndMakeVisible(_relativeOctaveSelector);
        _relativeOctaveSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
        _relativeOctaveSelector.addItem("-3", static_cast<int>(ETinTinTVoiceOctave::MinusThree));
        _relativeOctaveSelector.addItem("-2", static_cast<int>(ETinTinTVoiceOctave::MinusTwo));
        _relativeOctaveSelector.addItem("-1", static_cast<int>(ETinTinTVoiceOctave::MinusOne));
        _relativeOctaveSelector.addItem("0", static_cast<int>(ETinTinTVoiceOctave::Zero));
        _relativeOctaveSelector.addItem("1", static_cast<int>(ETinTinTVoiceOctave::One));
        _relativeOctaveSelector.addItem("2", static_cast<int>(ETinTinTVoiceOctave::Two));
        _relativeOctaveSelector.addItem("3", static_cast<int>(ETinTinTVoiceOctave::Three));
        _relativeOctaveSelector.setSelectedId(static_cast<int>(tin_tin::defaults::tVoiceFollowingOctave));
        _relativeOctaveSelector.onChange = [&]() -> void
        {
            _octaveRef.relativeOctave = static_cast<ETinTinTVoiceOctave>(
                _relativeOctaveSelector.getSelectedId() - static_cast<int>(ETinTinTVoiceOctave::Zero)
            );
        };

        // TODO: This is not matching logics octaves!
        addAndMakeVisible(_staticOctaveSelector);
        _staticOctaveSelector.setVisible(false);
        _staticOctaveSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
        _staticOctaveSelector.addItem("0", static_cast<int>(ETinTinTVoiceOctave::Zero));
        _staticOctaveSelector.addItem("1", static_cast<int>(ETinTinTVoiceOctave::One));
        _staticOctaveSelector.addItem("2", static_cast<int>(ETinTinTVoiceOctave::Two));
        _staticOctaveSelector.addItem("3", static_cast<int>(ETinTinTVoiceOctave::Three));
        _staticOctaveSelector.addItem("4", static_cast<int>(ETinTinTVoiceOctave::Four));
        _staticOctaveSelector.addItem("5", static_cast<int>(ETinTinTVoiceOctave::Five));
        _staticOctaveSelector.addItem("6", static_cast<int>(ETinTinTVoiceOctave::Six));
        _staticOctaveSelector.addItem("7", static_cast<int>(ETinTinTVoiceOctave::Seven));
        _staticOctaveSelector.addItem("8", static_cast<int>(ETinTinTVoiceOctave::Eight));
        _staticOctaveSelector.addItem("9", static_cast<int>(ETinTinTVoiceOctave::Nine));
        _staticOctaveSelector.setSelectedId(static_cast<int>(tin_tin::defaults::tVoiceStaticOctave));
        _staticOctaveSelector.onChange = [&]() -> void
        {
            _octaveRef.staticOctave = static_cast<ETinTinTVoiceOctave>(
                _staticOctaveSelector.getSelectedId() - static_cast<int>(ETinTinTVoiceOctave::Zero)
            );
        };
    }

    void setupMakeStaticToggle()
    {
        addAndMakeVisible(_makeStaticToggle);
        constexpr int positionX = octave_consts::padding + 120;
        constexpr int positionY = octave_consts::padding - 4;
        constexpr int width = 100;
        constexpr int height = 30;

        _makeStaticToggle.setBounds(
            positionX,
            positionY,
            width,
            height
        );

        _makeStaticToggle.onClick = [&]() -> void
        {
            if (_makeStaticToggle.getToggleState())
            {
                _staticOctaveSelector.setVisible(true);
                _relativeOctaveSelector.setVisible(false);
                _octaveRef.isStatic = true;
            }
            else
            {
                _staticOctaveSelector.setVisible(false);
                _relativeOctaveSelector.setVisible(true);
                _octaveRef.isStatic = false;
            }
        };
    }
};

class TinTinOctaveComponent final : public juce::Component
{
public:
    explicit TinTinOctaveComponent(PluginProcessor& processor, int xPosition = 0, int yPosition = 0) :
        _processorRef(processor)
    {

        setBounds(
            xPosition,
            yPosition,
            OctaveComponent::OCTAVE_COMPONENT_WIDTH,
            OctaveComponent::OCTAVE_COMPONENT_HEIGHT * 2
        );

        addAndMakeVisible(_sVoiceOctaveComponent);
        addAndMakeVisible(_iVoiceOctaveComponent);

        const auto bounds = _sVoiceOctaveComponent.getLocalBounds();
        const auto width = bounds.getWidth();
        const auto height = bounds.getHeight();
        _sVoiceOctaveComponent.setBounds(0, 0, width, height);
        _iVoiceOctaveComponent.setBounds(0, height, width, height);
    }

    ~TinTinOctaveComponent() override = default;

    inline void setInferiorVoiceEnabled(bool isEnabled)
    {
        _iVoiceOctaveComponent.setEnabled(isEnabled);
    }

    inline void setSuperiorVoiceEnabled(bool isEnabled)
    {
        _sVoiceOctaveComponent.setEnabled(isEnabled);
    }

private:
    PluginProcessor& _processorRef;
    OctaveComponent _sVoiceOctaveComponent{ _processorRef, _processorRef.tinTinProcessor.superiorOctave, "S-Voice:"};
    OctaveComponent _iVoiceOctaveComponent{ _processorRef, _processorRef.tinTinProcessor.inferiorOctave, "I-Voice:"};
};

