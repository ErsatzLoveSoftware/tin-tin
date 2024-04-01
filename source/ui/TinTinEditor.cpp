#include "TinTinEditor.h"
#include "../processors/TinTinProcessor.h"
#include "../WammyHelpers.h"

namespace tin_tin::editor_consts
{
    constexpr const char* TIN_TIN_NAME = "Tin Tin";

    // Plugin dimensions.
    constexpr int WIDTH = 550;
    constexpr int HEIGHT = 300;
    
    constexpr int PARENT_PADDING = 20;
}

using namespace tin_tin::editor_consts;

TinTinEditor::TinTinEditor(PluginProcessor& p) :
    AudioProcessorEditor(&p),
    _processorRef(p)
{
#if DEBUG
//    setupGUI_DebugInspector();
#endif // DEBUG

    auto bounds = getBounds(); // TODO: Use.

//    addAndMakeVisible(_bypassToggle);
    constexpr int bypassPositionX = 370;
    constexpr int bypassPositionY = 29;
    constexpr int bypassWidth = 80;
    constexpr int bypassHeight = 27;

    _bypassToggle.setBounds(
        bypassPositionX,
        bypassPositionY,
        bypassWidth,
        bypassHeight
    );

    _bypassToggle.onClick = [&]() -> void
    {
        _processorRef.tinTinProcessor.toggleBypass();
    };

    setupPanicButton();
    setupTriadRootComboBox();
    setupTriadTypeComboBox();
    setupTVoiceDirectionComboBox();
    setupTVoicePositionComboBox();

    addAndMakeVisible(_octaveComponent);
    _octaveComponent.setBounds(140, 107, 300, 80);

//    addAndMakeVisible(_noteDisplayComponent);
    _noteDisplayComponent.setTriad(_processorRef.tinTinProcessor.selectedTriad);

    setSize(tin_tin::editor_consts::WIDTH, tin_tin::editor_consts::HEIGHT);
}

void TinTinEditor::paint(juce::Graphics& g)
{
    juce::Rectangle<int> bounds = getLocalBounds();
    g.fillAll(juce::Colours::black); // Background color.

    // Title text.
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText(
        tin_tin::editor_consts::TIN_TIN_NAME,
        bounds.removeFromTop(50),
        juce::Justification::centred,
        false
    );

    constexpr int directionX = 0 + PARENT_PADDING;
    constexpr int directionY = 88;
    const auto directionLabelBounds = juce::Rectangle<int>(directionX, directionY, 60, 40);
    g.setFont(14.0f);
    g.drawText(
        "Direction:",
        directionLabelBounds,
        juce::Justification::centred,
        false
    );

    constexpr int positionX = 0 + PARENT_PADDING;
    constexpr int positionY = 143;
    const auto positionLabelBounds = juce::Rectangle<int>(positionX, positionY, 60, 40);
    g.setFont(14.0f);
    g.drawText(
        "Position:",
        positionLabelBounds,
        juce::Justification::centred,
        false
    );

    _octaveComponent.paint(g);
    // TODO investigate parameters.
//    _noteDisplayComponent.setTriad(_processorRef.tinTinProcessor.selectedTriad);
//    _noteDisplayComponent.setBounds(bounds.removeFromBottom(200));
}

void TinTinEditor::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromBottom(PARENT_PADDING);
    bounds.removeFromTop(PARENT_PADDING);
    bounds.removeFromRight(PARENT_PADDING);
    bounds.removeFromLeft(PARENT_PADDING);

    _panicButton.setBounds(bounds.removeFromRight(80).removeFromTop(40));

#if DEBUG
    inspectButton.setBounds(getLocalBounds().withSizeKeepingCentre(100, 50));
#endif // DEBUG
}

void TinTinEditor::setupPanicButton()
{
    addAndMakeVisible(_panicButton);
    _panicButton.setColour(
        juce::TextButton::ColourIds::buttonColourId,
        juce::Colour::fromRGB(0, 0, 0)
    );

    // Register callbacks.
    _panicButton.onClick = [&]() -> void
    {
        _processorRef.tinTinProcessor.panic();
    };
}

void TinTinEditor::setupTriadRootComboBox()
{
    constexpr int selectorWidth = 60;
    constexpr int selectorHeight = 20;
    constexpr int selectorPositionX = 0 + PARENT_PADDING;
    constexpr int selectorPositionY = 65;

    addAndMakeVisible(_triadRootSelector);
    _triadRootSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    for (MidiNote note = 1; note < wammy::consts::NUM_SEMI_TONES_IN_OCTAVE + 1; ++note)
    {
        _triadRootSelector.addItem(wammy::audio_utils::stringifyMidiNote(note - 1).data(), note);
    }

    _triadRootSelector.setSelectedId(
        static_cast<int>(tin_tin::defaults::triadRoot) + 1
    );

    // Changed callback.
    _triadRootSelector.onChange = [&]() -> void
    {
        _processorRef.tinTinProcessor.updateVoiceCacheMap(
            static_cast<wammy::audio_utils::ENote>(_triadRootSelector.getSelectedId() - 1)
        );
    };
}

void TinTinEditor::setupTriadTypeComboBox()
{
    constexpr int selectorWidth = 118;
    constexpr int selectorHeight = 20;
    constexpr int selectorPositionX = 0 + PARENT_PADDING;
    constexpr int selectorPositionY = 35;

    addAndMakeVisible(_triadSelector);
    _triadSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _triadSelector.addItem("Major", static_cast<int>(ETinTinTriadType::Major));
    _triadSelector.addItem("Minor", static_cast<int>(ETinTinTriadType::Minor));
    _triadSelector.addItem("Augmented", static_cast<int>(ETinTinTriadType::Augmented));
    _triadSelector.addItem("Diminished", static_cast<int>(ETinTinTriadType::Diminished));
    _triadSelector.setSelectedId(static_cast<int>(tin_tin::defaults::triadType)); // Set default.
    _triadSelector.onChange = [&]() -> void
    {
        constexpr auto scale = std::nullopt;
        _processorRef.tinTinProcessor.updateVoiceCacheMap(
            scale,
            static_cast<ETinTinTriadType>(_triadSelector.getSelectedId())
        );
    };
}

void TinTinEditor::setupTVoiceDirectionComboBox()
{
    constexpr int selectorPositionX = 0 + PARENT_PADDING;
    constexpr int selectorPositionY = 120;
    constexpr int selectorWidth = 120;
    constexpr int selectorHeight = 20;

    addAndMakeVisible(_tVoiceDirectionSelector);
    _tVoiceDirectionSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _tVoiceDirectionSelector.addItem("Superior", static_cast<int>(ETinTinDirection::Superior));
    _tVoiceDirectionSelector.addItem("Inferior", static_cast<int>(ETinTinDirection::Inferior));
    _tVoiceDirectionSelector.addItem("Alternating", static_cast<int>(ETinTinDirection::Alternating));
    _tVoiceDirectionSelector.setSelectedId(static_cast<int>(tin_tin::defaults::tVoiceDirection));
    _tVoiceDirectionSelector.onChange = [&]() -> void
    {
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
            _octaveComponent.setInferiorVoiceEnabled(true);
            _octaveComponent.setSuperiorVoiceEnabled(true);
            break;
        }
    };
}

void TinTinEditor::setupTVoicePositionComboBox()
{
    constexpr int selectorPositionX = 0 + PARENT_PADDING;
    constexpr int selectorPositionY = 174;
    constexpr int selectorWidth = 120;
    constexpr int selectorHeight = 20;
    
    addAndMakeVisible(_tVoicePositionSelector);
    _tVoicePositionSelector.setBounds(selectorPositionX, selectorPositionY, selectorWidth, selectorHeight);
    _tVoicePositionSelector.addItem("First", static_cast<int>(ETinTinPosition::FirstPosition));
    _tVoicePositionSelector.addItem("Second", static_cast<int>(ETinTinPosition::SecondPosition));
    _tVoicePositionSelector.addItem("Alternating", static_cast<int>(ETinTinPosition::Alternating));
    _tVoicePositionSelector.setSelectedId(static_cast<int>(tin_tin::defaults::tVoicePosition));
    _tVoicePositionSelector.onChange = [&]() -> void
    {
        _processorRef.tinTinProcessor.tVoicePosition = static_cast<ETinTinPosition>(
            _tVoicePositionSelector.getSelectedId()
        );
    };
}

#if DEBUG
void TinTinEditor::setupGUI_DebugInspector()
{
    addAndMakeVisible(inspectButton);
    inspectButton.setBounds(
        0,
        tin_tin::editor_consts::HEIGHT - 10,
        80,
        40
    );

    inspectButton.onClick = [&]() -> void
    {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector>(*this);
            inspector->onClose = [this]() -> void
            {
                inspector.reset();
            };
        }

        inspector->setVisible(true);
    };
}
#endif // DEBUG
