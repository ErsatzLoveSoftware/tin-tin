#pragma once

#include "../processors/PluginProcessor.h"
#include "BinaryData.h"

#include "juce_gui_extra/juce_gui_extra.h"
#include "juce_gui_basics/juce_gui_basics.h"

#include "TinTinComponents.h"
#include "TinTInNoteDisplayComponent.h"

#if DEBUG
    #include "melatonin_inspector/melatonin_inspector.h"
    #include "TinTinOctaveComponent.h"
#endif // DEBUG

class TinTinEditor final : public juce::AudioProcessorEditor
{
public:
    explicit TinTinEditor(
        PluginProcessor& p,
        juce::AudioProcessorValueTreeState& paramTree
    );

    ~TinTinEditor() override
    {
        _tVoiceVelocityAttachment.reset(); // TODO: removed this to default.
    }
    
    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // Reference to audio/midi processor.
    PluginProcessor& _processorRef;
    juce::AudioProcessorValueTreeState& _paramTree;

    // Master controls.
    std::unique_ptr<ButtonAttachment> _bypassAttachment;
    juce::ToggleButton _bypassToggle{ "bypass" };
    juce::ToggleButton _muteMVoiceToggle{ "mute m voice :x" };

    // tintin Controls.
    std::unique_ptr<ComboBoxAttachment> _triadComboBoxAttachment;
    TinTinComboBox _triadSelector{"triad"};
    
    std::unique_ptr<ComboBoxAttachment> _rootComboBoxAttachment;
    TinTinComboBox _triadRootSelector{"scale root selector"};
    
    std::unique_ptr<ComboBoxAttachment> _directionComboBoxAttachment;
    TinTinComboBox _tVoiceDirectionSelector{"t voice direction"};
    
    std::unique_ptr<ComboBoxAttachment> _positionComboBoxAttachment;
    TinTinComboBox _tVoicePositionSelector{"t voice position"};
    
    // TODO: Logic for octave switching.
    TinTinOctaveComponent _octaveComponent{_processorRef};

    // Midi Controls.
    TinTinButton _panicButton{ "panic!! :O" };
    TinTinComboBox _tVoiceMidiChannelSelector{"midi channel"};
    
    std::unique_ptr<SliderAttachment> _tVoiceVelocityAttachment;
    juce::Slider _tVoiceVelocitySlider{"t voice velocity"};
    
    // Displays
    TinTinNoteDisplayComponent _noteDisplayComponent;

    void setupBypassToggle();
    void setupPanicButton();
    void setupTriadRootComboBox();
    void setupTriadTypeComboBox();
    void setupTVoiceDirectionComboBox();
    void setupTVoicePositionComboBox();
    void setupMVoiceMuteToggle();
    void setupTVoiceVelocitySlider();
    void setupTMidiChannelSelector();

#if DEBUG
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton{ "Debug UI" };
    [[maybe_unused]] void setupGUI_DebugInspector();
#endif // DEBUG

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TinTinEditor)
};
