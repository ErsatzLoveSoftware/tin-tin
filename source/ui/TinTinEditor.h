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
    TinTinComboBox _triadRootSelector{ ""};
    TinTinComboBox _tVoiceDirectionSelector{""};
    TinTinComboBox _tVoicePositionSelector{""};
    TinTinComboBox _triadSelector{""};

    TinTinOctaveComponent _octaveComponent{_processorRef};
    TinTinNoteDisplayComponent _noteDisplayComponent;

private:
    void setupPanicButton();
    void setupTriadRootComboBox();
    void setupTriadTypeComboBox();
    void setupTVoiceDirectionComboBox();
    void setupTVoicePositionComboBox();
    void setupMVoiceMuteToggle();

#if DEBUG
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton{ "Debug UI" };
    void setupGUI_DebugInspector();
#endif // DEBUG

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TinTinEditor)
};
