#include "PluginProcessor.h"
#include "../ui/TinTinEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
    .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
    .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
), paramTree(*this, nullptr, "Params", createParameterLayout()) {
}

PluginProcessor::~PluginProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // ::::::: Bypass ::::::: 
    auto bypass = std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false);

    // ::::::: Triad Root :::::::
    juce::StringArray triadRootsArray;
    for (MidiNote note = 1; note < wammy::consts::NUM_SEMI_TONES_IN_OCTAVE + 1; ++note)
    {
        triadRootsArray.add(wammy::audio_utils::stringifyMidiNote(note - 1).data());
    }
    
    auto triadRoots = std::make_unique<juce::AudioParameterChoice>(
        "scale root selector", "Scale Root Selector", triadRootsArray, 0
    );

    // ::::::: Triad Type :::::::
    juce::StringArray triadTypesArray;
    triadTypesArray.add("major");
    triadTypesArray.add("minor");
    triadTypesArray.add("augmented");
    triadTypesArray.add("diminished");
    
    auto triadTypes = std::make_unique<juce::AudioParameterChoice>(
        "triad", "Triad", triadTypesArray, 0
    );
    
    // ::::::: Direction Algo :::::::
    juce::StringArray directionsArray;
    directionsArray.add("superior");
    directionsArray.add("inferior");
    directionsArray.add("alternating");
    directionsArray.add("follow m voice");
    directionsArray.add("counter m voice");
    
    auto directionTypes = std::make_unique<juce::AudioParameterChoice>(
        "t voice direction", "T Voice Direction", directionsArray, 0
    );
    
    // ::::::: Position Algo :::::::
    juce::StringArray positionsArray;
    positionsArray.add("first");
    positionsArray.add("second");
    positionsArray.add("alternating");
    
    auto positionTypes = std::make_unique<juce::AudioParameterChoice>(
        "t voice position", "T Voice Position", positionsArray, 0
    );
    
    // ::::::: Position Algo :::::::
    juce::StringArray octaveArray;
    octaveArray.add("-3");
    octaveArray.add("-2");
    octaveArray.add("-1");
    octaveArray.add("0");
    octaveArray.add("1");
    octaveArray.add("2");
    octaveArray.add("3");
    
    // TODO: Change options when make static is pressed.
    auto octavePositions = std::make_unique<juce::AudioParameterChoice>(
        "s voice octave", "S Voice Octave", octaveArray, 0
    );
    
//    juce::AudioParameterChoice::Listener listener;
//    octavePositions->addListener();

    // ::::::: T Voice Velocity :::::::
    auto tVoiceVelocitySlider = std::make_unique<juce::AudioParameterFloat>(
        "t voice velocity", "T Voice Velocity", 0.f, 1.f, static_cast<float>(tin_tin::defaults::tVoiceVelocity)
    );
    
    params.push_back(std::move(bypass));
    params.push_back(std::move(triadRoots));
    params.push_back(std::move(triadTypes));
    params.push_back(std::move(directionTypes));
    params.push_back(std::move(positionTypes));
    params.push_back(std::move(tVoiceVelocitySlider));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram() {
    return 0;
}

void PluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void PluginProcessor::processBlock([[maybe_unused]] juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    tinTinProcessor.process(midiMessages);
}

//==============================================================================
bool PluginProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginProcessor::createEditor() {
    return new TinTinEditor(*this, paramTree);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE
createPluginFilter() {
    return new PluginProcessor();
}
