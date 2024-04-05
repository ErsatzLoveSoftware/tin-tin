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

    // UI components.
    TinTinButton _panicButton{ "panic!! :O" };
    juce::ToggleButton _bypassToggle{ "bypass" };
    juce::ToggleButton _muteMVoiceToggle{ "mute m voice :x" };
    juce::Slider _tVoiceVelocitySlider{"T Voice Velocity"};
    TinTinComboBox _triadRootSelector{"Scale Root Selector"};
    TinTinComboBox _tVoiceDirectionSelector{"T Voice Direction"};
    TinTinComboBox _tVoicePositionSelector{"T Voice Position"};
    TinTinComboBox _triadSelector{"Triad"};

    TinTinOctaveComponent _octaveComponent{_processorRef};
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

#if DEBUG
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton{ "Debug UI" };
    void setupGUI_DebugInspector();
#endif // DEBUG

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TinTinEditor)
};
