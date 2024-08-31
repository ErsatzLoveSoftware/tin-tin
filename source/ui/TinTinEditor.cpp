#include "TinTinEditor.h"

#include <memory>

namespace tin_tin::editor_consts
{
    // Editor Name.
    constexpr const char* TIN_TIN_NAME = "Tin Tin";

    // Plugin dimensions.
    constexpr int WIDTH = 550;
    constexpr int HEIGHT = 300;
    constexpr int PARENT_PADDING = 20;
}

TinTinEditor::TinTinEditor (
    PluginProcessor& p,
    juce::AudioProcessorValueTreeState& paramTree) : AudioProcessorEditor (&p),
                                                     _processorRef (p),
                                                     _paramTree (paramTree)
{
#if DEBUG
//    setupGUI_DebugInspector();
#endif // DEBUG

    [[maybe_unused]] juce::Rectangle<int> bounds = getBounds(); // TODO: Use.

    setupBypassToggle();
    setupPanicButton();
    setupTriadRootComboBox();
    setupTriadTypeComboBox();
    setupTVoiceDirectionComboBox();
    setupTVoicePositionComboBox();
    setupMVoiceMuteToggle();
    setupTVoiceVelocitySlider();
    setupTMidiChannelSelector();

    addAndMakeVisible (_octaveComponent);
    _octaveComponent.setBounds (140, 107, 300, 80);

    _noteDisplayComponent.setTriad (_processorRef.tinTinProcessor.selectedTriad);

    setSize (tin_tin::editor_consts::WIDTH, tin_tin::editor_consts::HEIGHT);
}

void TinTinEditor::paint (juce::Graphics& g)
{
    juce::Rectangle<int> bounds = getLocalBounds();
    g.fillAll (juce::Colours::black); // Background color.

    // Title text.
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawText (
        tin_tin::editor_consts::TIN_TIN_NAME,
        bounds.removeFromTop (50),
        juce::Justification::centred,
        false);

    constexpr int directionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int directionY = 88;
    const auto directionLabelBounds = juce::Rectangle<int> (directionX, directionY, 60, 40);
    g.setFont (14.0f);
    g.drawText (
        "direction:",
        directionLabelBounds,
        juce::Justification::centred,
        false);

    constexpr int positionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int positionY = 143;
    const auto positionLabelBounds = juce::Rectangle<int> (positionX, positionY, 60, 40);
    g.setFont (14.0f);
    g.drawText (
        "position:",
        positionLabelBounds,
        juce::Justification::centred,
        false);

    _octaveComponent.paint (g);
}

void TinTinEditor::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromBottom (tin_tin::editor_consts::PARENT_PADDING);
    bounds.removeFromTop (tin_tin::editor_consts::PARENT_PADDING);
    bounds.removeFromRight (tin_tin::editor_consts::PARENT_PADDING);
    bounds.removeFromLeft (tin_tin::editor_consts::PARENT_PADDING);

    _panicButton.setBounds (bounds.removeFromRight (80).removeFromTop (40));

#if DEBUG
    inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre (100, 50));
#endif // DEBUG
}

void TinTinEditor::setupBypassToggle()
{
    addAndMakeVisible (_bypassToggle);
    constexpr int bypassPositionX = 370;
    constexpr int bypassPositionY = 29;
    constexpr int bypassWidth = 80;
    constexpr int bypassHeight = 27;

    _bypassToggle.setBounds (
        bypassPositionX,
        bypassPositionY,
        bypassWidth,
        bypassHeight);

    _bypassToggle.onClick = [&]() -> void {
        _processorRef.tinTinProcessor.toggleBypass();

        if (_bypassToggle.getToggleState() && _muteMVoiceToggle.getToggleState())
        {
            _muteMVoiceToggle.setColour (
                juce::ToggleButton::ColourIds::textColourId,
                juce::Colour::fromRGB (255, 0, 40));

            _muteMVoiceToggle.repaint();
        }
        else
        {
            _muteMVoiceToggle.setColour (
                juce::ToggleButton::ColourIds::textColourId,
                juce::Colour::fromRGB (255, 255, 255));

            _muteMVoiceToggle.repaint();
        }
    };

    _bypassAttachment = std::make_unique<ButtonAttachment> (_paramTree, "bypass", _bypassToggle);
}

void TinTinEditor::setupPanicButton()
{
    addAndMakeVisible (_panicButton);
    _panicButton.setColour (
        juce::TextButton::ColourIds::buttonColourId,
        juce::Colour::fromRGB (0, 0, 0));

    // Register callbacks.
    _panicButton.onClick = [&]() -> void {
        _processorRef.tinTinProcessor.panic();
    };
}

void TinTinEditor::setupTriadRootComboBox()
{
    constexpr int selectorWidth = 60;
    constexpr int selectorHeight = 20;
    constexpr int selectorPositionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 65;

    addAndMakeVisible (_triadRootSelector);
    _triadRootSelector.setBounds (selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    for (MidiNote note = 1; note < wammy::consts::NUM_SEMI_TONES_IN_OCTAVE + 1; ++note)
    {
        _triadRootSelector.addItem(wammy::audio_utils::stringifyMidiNote(note - 1).data(), note);
    }

    _triadRootSelector.setSelectedId (
        static_cast<int> (tin_tin::defaults::triadRoot) + 1);

    // Changed callback.
    _triadRootSelector.onChange = [&]() -> void {
        _processorRef.tinTinProcessor.updateVoiceCacheMap (
            static_cast<wammy::audio_utils::ENote> (_triadRootSelector.getSelectedId() - 1));
    };

    _rootComboBoxAttachment = std::make_unique<ComboBoxAttachment> (_paramTree, "scale root selector", _triadRootSelector);
}

void TinTinEditor::setupTriadTypeComboBox()
{
    constexpr int selectorWidth = 118;
    constexpr int selectorHeight = 20;
    constexpr int selectorPositionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 35;

    addAndMakeVisible (_triadSelector);
    _triadSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _triadSelector.addItem("major", static_cast<int>(ETinTinTriadType::Major));
    _triadSelector.addItem("minor", static_cast<int>(ETinTinTriadType::Minor));
    _triadSelector.addItem("augmented", static_cast<int>(ETinTinTriadType::Augmented));
    _triadSelector.addItem("diminished", static_cast<int>(ETinTinTriadType::Diminished));
    _triadSelector.setSelectedId (static_cast<int>(tin_tin::defaults::triadType)); // Set default.
    _triadSelector.onChange = [&]() -> void {
        constexpr auto scale = std::nullopt;
        _processorRef.tinTinProcessor.updateVoiceCacheMap(
            scale,
            static_cast<ETinTinTriadType>(_triadSelector.getSelectedId()));
    };

    _triadComboBoxAttachment = std::make_unique<ComboBoxAttachment> (_paramTree, "triad", _triadSelector);
}

void TinTinEditor::setupTVoiceDirectionComboBox()
{
    constexpr int selectorPositionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 120;
    constexpr int selectorWidth = 130;
    constexpr int selectorHeight = 20;

    addAndMakeVisible (_tVoiceDirectionSelector);
    _tVoiceDirectionSelector.setBounds (selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _tVoiceDirectionSelector.addItem ("superior", static_cast<int>(ETinTinDirection::Superior));
    _tVoiceDirectionSelector.addItem ("inferior", static_cast<int>(ETinTinDirection::Inferior));
    _tVoiceDirectionSelector.addItem ("alternating", static_cast<int>(ETinTinDirection::Alternating));
    _tVoiceDirectionSelector.addItem ("follow m voice", static_cast<int>(ETinTinDirection::FollowMVoiceDirection));
    _tVoiceDirectionSelector.addItem ("counter m voice", static_cast<int>(ETinTinDirection::CounterMVoiceDirection));
    _tVoiceDirectionSelector.setSelectedId (static_cast<int>(tin_tin::defaults::tVoiceDirection));
    _tVoiceDirectionSelector.onChange = [&]() -> void {
        auto direction = static_cast<ETinTinDirection>(_tVoiceDirectionSelector.getSelectedId());
        _processorRef.tinTinProcessor.tVoiceDirection = direction;

        switch (direction)
        {
            case (ETinTinDirection::Inferior):
                _octaveComponent.setInferiorVoiceEnabled(true);
                _octaveComponent.setSuperiorVoiceEnabled(false);
                break;

            case (ETinTinDirection::Superior):
                _octaveComponent.setInferiorVoiceEnabled(false);
                _octaveComponent.setSuperiorVoiceEnabled(true);
                break;

            case (ETinTinDirection::Alternating):
            case (ETinTinDirection::FollowMVoiceDirection):
            case (ETinTinDirection::CounterMVoiceDirection):
                _octaveComponent.setInferiorVoiceEnabled(true);
                _octaveComponent.setSuperiorVoiceEnabled(true);
                break;
        }
    };

    _directionComboBoxAttachment = std::make_unique<ComboBoxAttachment> (_paramTree, "t voice direction", _tVoiceDirectionSelector);
}

void TinTinEditor::setupTVoicePositionComboBox()
{
    constexpr int selectorPositionX = 0 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 174;
    constexpr int selectorWidth = 120;
    constexpr int selectorHeight = 20;

    addAndMakeVisible (_tVoicePositionSelector);
    _tVoicePositionSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _tVoicePositionSelector.addItem("first", static_cast<int> (ETinTinPosition::FirstPosition));
    _tVoicePositionSelector.addItem("second", static_cast<int> (ETinTinPosition::SecondPosition));
    _tVoicePositionSelector.addItem("alternating", static_cast<int> (ETinTinPosition::Alternating));
    _tVoicePositionSelector.setSelectedId(static_cast<int> (tin_tin::defaults::tVoicePosition));
    _tVoicePositionSelector.onChange = [&]() -> void {
        _processorRef.tinTinProcessor.tVoicePosition = static_cast<ETinTinPosition>(
            _tVoicePositionSelector.getSelectedId());
    };
    
    _positionComboBoxAttachment = std::make_unique<ComboBoxAttachment> (_paramTree, "t voice position", _tVoicePositionSelector);
}

void TinTinEditor::setupMVoiceMuteToggle()
{
    constexpr int selectorPositionX = 140 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 190;
    constexpr int selectorWidth = 200;
    constexpr int selectorHeight = 20;

    addAndMakeVisible (_muteMVoiceToggle);
    _muteMVoiceToggle.setBounds (selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _muteMVoiceToggle.onStateChange = [&]() -> void {
        _processorRef.tinTinProcessor.toggleMuteMVoice();

        if (!_muteMVoiceToggle.getToggleState())
        {
            _muteMVoiceToggle.setColour (
                juce::ToggleButton::ColourIds::textColourId,
                juce::Colour::fromRGB (255, 255, 255));

            _muteMVoiceToggle.repaint();
        }
        else if (_muteMVoiceToggle.getToggleState() && _bypassToggle.getToggleState())
        {
            _muteMVoiceToggle.setColour(
                juce::ToggleButton::ColourIds::textColourId,
                juce::Colour::fromRGB(255, 0, 40));

            _muteMVoiceToggle.repaint();
        }
    };
}

void TinTinEditor::setupTVoiceVelocitySlider()
{
    constexpr int selectorPositionX = 140 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 230;
    constexpr int selectorWidth = 200;
    constexpr int selectorHeight = 40;

    addAndMakeVisible (_tVoiceVelocitySlider);
    _tVoiceVelocitySlider.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    constexpr double min = 0.;
    constexpr double max = 1.;
    constexpr double increment = .01;
    _tVoiceVelocitySlider.setRange(min, max, increment);
    _tVoiceVelocitySlider.setValue(tin_tin::defaults::tVoiceVelocity);
    _tVoiceVelocitySlider.onValueChange = [&]() -> void {
        _processorRef.tinTinProcessor.updateTVoiceVelocity(
            static_cast<float>(_tVoiceVelocitySlider.getValue()));
    };
    
    _tVoiceVelocityAttachment = std::make_unique<SliderAttachment>(_paramTree, "t voice velocity", _tVoiceVelocitySlider);
}

void TinTinEditor::setupTMidiChannelSelector()
{
    addAndMakeVisible (_tVoiceMidiChannelSelector);
    constexpr int selectorPositionX = 400 + tin_tin::editor_consts::PARENT_PADDING;
    constexpr int selectorPositionY = 230;
    constexpr int selectorWidth = 70;
    constexpr int selectorHeight = 20;

    addAndMakeVisible (_tVoiceMidiChannelSelector);
    _tVoiceMidiChannelSelector.setBounds (selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _tVoiceMidiChannelSelector.addItem ("1", 1);
    _tVoiceMidiChannelSelector.addItem ("2", 2);
    _tVoiceMidiChannelSelector.addItem ("3", 3);
    _tVoiceMidiChannelSelector.addItem ("4", 4);
    _tVoiceMidiChannelSelector.addItem ("5", 5);
    _tVoiceMidiChannelSelector.addItem ("6", 6);
    _tVoiceMidiChannelSelector.addItem ("7", 7);
    _tVoiceMidiChannelSelector.addItem ("8", 8);
    _tVoiceMidiChannelSelector.addItem ("9", 9);
    _tVoiceMidiChannelSelector.addItem ("10", 10);
    _tVoiceMidiChannelSelector.addItem ("11", 11);
    _tVoiceMidiChannelSelector.addItem ("12", 12);
    _tVoiceMidiChannelSelector.addItem ("13", 13);
    _tVoiceMidiChannelSelector.addItem ("14", 14);
    _tVoiceMidiChannelSelector.addItem ("15", 15);
    _tVoiceMidiChannelSelector.addItem ("16", 16);
    _tVoiceMidiChannelSelector.setSelectedId (tin_tin::defaults::tVoiceMidiChannel);
    _tVoiceMidiChannelSelector.onChange = [&]() -> void {
        _processorRef.tinTinProcessor.updateTVoiceMidiChannel (
            _tVoiceMidiChannelSelector.getSelectedId());
    };
}

#if DEBUG
[[maybe_unused]] void TinTinEditor::setupGUI_DebugInspector()
{
    addAndMakeVisible (inspectButton);
    inspectButton.setBounds (
        0,
        tin_tin::editor_consts::HEIGHT - 10,
        80,
        40);

    inspectButton.onStateChange = [&]() -> void {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() -> void {
                inspector.reset();
            };
        }

        inspector->setVisible (true);
    };
}
#endif // DEBUG
