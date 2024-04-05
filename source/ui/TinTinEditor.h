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

class TinTinEditor final :
    public juce::AudioProcessorEditor
{
public:
    explicit TinTinEditor(PluginProcessor&);

    ~TinTinEditor() override = default;

    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    // Reference to audio/midi processor.
    PluginProcessor& _processorRef;

    // Master controls.
    juce::ToggleButton _bypassToggle{ "bypass" };
    juce::ToggleButton _muteMVoiceToggle{ "mute m voice :x" };

    // tintin Controls.
    TinTinComboBox _triadSelector{"triad"};
    TinTinComboBox _triadRootSelector{"scale root selector"};
    TinTinComboBox _tVoiceDirectionSelector{"t voice direction"};
    TinTinComboBox _tVoicePositionSelector{"t voice position"};
    TinTinOctaveComponent _octaveComponent{_processorRef};

    // Midi Controls.
    TinTinButton _panicButton{ "panic!! :O" };
    juce::Slider _tVoiceVelocitySlider{"t voice velocity"};
    TinTinComboBox _tVoiceMidiChannelSelector{"midi channel"};
    
    // Displays
    TinTinNoteDisplayComponent _noteDisplayComponent;

private:
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
    void setupGUI_DebugInspector();
#endif // DEBUG

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TinTinEditor)
};
