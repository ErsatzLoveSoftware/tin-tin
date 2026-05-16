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
), paramTree(*this, nullptr, "Params", createParameterLayout())
{
}

PluginProcessor::~PluginProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ::::::: Bypass :::::::
    auto bypass = std::make_unique<juce::AudioParameterBool>("bypass", "Bypass", false);
    params.push_back(std::move(bypass));

    // ::::::: Triad Root :::::::
    juce::StringArray triadRootsArray;
    for (MidiNote note = 1; note < wammy::consts::NUM_SEMI_TONES_IN_OCTAVE + 1; ++note)
    {
        triadRootsArray.add(wammy::audio_utils::stringifyMidiNote(note - 1).data());
    }

    auto triadRoots = std::make_unique<juce::AudioParameterChoice>("scale root selector",
        "Scale Root Selector",
        triadRootsArray,
        0);

    params.push_back(std::move(triadRoots));

    // ::::::: Triad Type :::::::
    juce::StringArray triadTypesArray;
    triadTypesArray.add("major");
    triadTypesArray.add("minor");
    triadTypesArray.add("augmented");
    triadTypesArray.add("diminished");

    auto triadTypes = std::make_unique<juce::AudioParameterChoice>(
        "triad", "Triad", triadTypesArray, 0
    );
    params.push_back(std::move(triadTypes));

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
    params.push_back(std::move(directionTypes));

    // ::::::: Position Algo :::::::
    juce::StringArray positionsArray;
    positionsArray.add("first");
    positionsArray.add("second");
    positionsArray.add("alternating");

    auto positionTypes = std::make_unique<juce::AudioParameterChoice>(
        "t voice position", "T Voice Position", positionsArray, 0
    );

    params.push_back(std::move(positionTypes));

    // ::::::: Position Algo :::::::
    juce::StringArray octaveArray;
    octaveArray.add("-3");
    octaveArray.add("-2");
    octaveArray.add("-1");
    octaveArray.add("0");
    octaveArray.add("1");
    octaveArray.add("2");
    octaveArray.add("3");

    // ::::::: Octave Selector :::::::
    auto octavePositions = std::make_unique<juce::AudioParameterChoice>(
        "s voice octave", "S Voice Octave", octaveArray, 0
    );

//    octavePositions->addListener(this);
    params.push_back(std::move(octavePositions));

    // ::::::: T Voice Velocity :::::::
    auto tVoiceVelocitySlider = std::make_unique<juce::AudioParameterFloat>(
        "t voice velocity", "T Voice Velocity", 0.f, 1.f, static_cast<float>(tin_tin::defaults::tVoiceVelocity)
    );

    params.push_back(std::move(tVoiceVelocitySlider));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String PluginProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PluginProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
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

void PluginProcessor::processBlock([[maybe_unused]] juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    tinTinProcessor.process(midiMessages);
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new TinTinEditor(*this, paramTree);
}

void PluginProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::XmlElement root("TinTin2State");

    // APVTS covers: bypass, scale root, triad type, direction, position, velocity
    root.addChildElement(paramTree.copyState().createXml().release());

    // Extra state not tracked by APVTS
    auto* extras = root.createNewChildElement("Extras");
    extras->setAttribute("midiChannel", tinTinProcessor.getMidiChannel());
    extras->setAttribute("muteMVoice",  tinTinProcessor.getMuteMVoice());

    auto* sup = extras->createNewChildElement("SuperiorOctave");
    sup->setAttribute("relativeOctave", static_cast<int>(tinTinProcessor.superiorOctave.relativeOctave));
    sup->setAttribute("staticOctave",   static_cast<int>(tinTinProcessor.superiorOctave.staticOctave));
    sup->setAttribute("isStatic",       tinTinProcessor.superiorOctave.isStatic);

    auto* inf = extras->createNewChildElement("InferiorOctave");
    inf->setAttribute("relativeOctave", static_cast<int>(tinTinProcessor.inferiorOctave.relativeOctave));
    inf->setAttribute("staticOctave",   static_cast<int>(tinTinProcessor.inferiorOctave.staticOctave));
    inf->setAttribute("isStatic",       tinTinProcessor.inferiorOctave.isStatic);

    copyXmlToBinary(root, destData);
}

void PluginProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml == nullptr || !xml->hasTagName("TinTin2State"))
        return;

    // Restore APVTS (updates attached UI controls when the editor next opens)
    if (auto* paramsXml = xml->getChildByName(paramTree.state.getType()))
        paramTree.replaceState(juce::ValueTree::fromXml(*paramsXml));

    // ComboBoxAttachment restores the display silently (dontSendNotification),
    // so onChange never fires — apply parameter values to the processor directly.
    auto getChoiceIndex = [&](const char* id) -> int
    {
        auto* p = dynamic_cast<juce::AudioParameterChoice*>(paramTree.getParameter(id));
        return p ? p->getIndex() : 0;
    };

    tinTinProcessor.updateVoiceCacheMap(
        static_cast<wammy::audio_utils::ENote>(getChoiceIndex("scale root selector")),
        static_cast<ETinTinTriadType>(getChoiceIndex("triad") + 1));

    tinTinProcessor.tVoiceDirection = static_cast<ETinTinDirection>(getChoiceIndex("t voice direction") + 1);
    tinTinProcessor.tVoicePosition  = static_cast<ETinTinPosition>(getChoiceIndex("t voice position") + 1);

    if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(paramTree.getParameter("t voice velocity")))
        tinTinProcessor.updateTVoiceVelocity(p->get());

    if (auto* p = dynamic_cast<juce::AudioParameterBool*>(paramTree.getParameter("bypass")))
        tinTinProcessor.setBypass(p->get());

    // Restore extras
    if (auto* extras = xml->getChildByName("Extras"))
    {
        tinTinProcessor.updateTVoiceMidiChannel(extras->getIntAttribute("midiChannel", 1));
        tinTinProcessor.setMuteMVoice(extras->getBoolAttribute("muteMVoice", false));

        if (auto* sup = extras->getChildByName("SuperiorOctave"))
        {
            tinTinProcessor.superiorOctave.relativeOctave = static_cast<ETinTinTVoiceOctave>(sup->getIntAttribute("relativeOctave", 0));
            tinTinProcessor.superiorOctave.staticOctave   = static_cast<ETinTinTVoiceOctave>(sup->getIntAttribute("staticOctave",   5));
            tinTinProcessor.superiorOctave.isStatic       = sup->getBoolAttribute("isStatic", false);
        }

        if (auto* inf = extras->getChildByName("InferiorOctave"))
        {
            tinTinProcessor.inferiorOctave.relativeOctave = static_cast<ETinTinTVoiceOctave>(inf->getIntAttribute("relativeOctave", 0));
            tinTinProcessor.inferiorOctave.staticOctave   = static_cast<ETinTinTVoiceOctave>(inf->getIntAttribute("staticOctave",   5));
            tinTinProcessor.inferiorOctave.isStatic       = inf->getBoolAttribute("isStatic", false);
        }
    }
}

void PluginProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    printf("Liba!!!!!!!!");
    printf("%i, %f", parameterIndex, newValue);
}

void PluginProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting)
{
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new PluginProcessor();
}
