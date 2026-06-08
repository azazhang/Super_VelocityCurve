#include "PluginProcessor.h"
#include "PluginEditor.h"

SuperVelocityCurveAudioProcessor::SuperVelocityCurveAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    profileStore.applyActiveToEngine (engine);
    apvts.addParameterListener ("outputMode", this);
}

SuperVelocityCurveAudioProcessor::~SuperVelocityCurveAudioProcessor()
{
    apvts.removeParameterListener ("outputMode", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout SuperVelocityCurveAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "outputMode",
        "Velocity Output",
        juce::StringArray { "Auto", "MIDI 1.0", "MIDI 2.0" },
        0));

    return { params.begin(), params.end() };
}

void SuperVelocityCurveAudioProcessor::parameterChanged (const juce::String& parameterID, float)
{
    if (parameterID == "outputMode")
        syncOutputModeToEngine();
}

void SuperVelocityCurveAudioProcessor::prepareToPlay (double sampleRate, int)
{
    engine.setSampleRate (sampleRate);
    syncOutputModeToEngine();
    applyProfileToEngine();
}

void SuperVelocityCurveAudioProcessor::releaseResources()
{
}

bool SuperVelocityCurveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::disabled()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void SuperVelocityCurveAudioProcessor::syncOutputModeToEngine()
{
    const auto outputModeIndex = static_cast<int> (*apvts.getRawParameterValue ("outputMode"));
    engine.setOutputMode (static_cast<svc::VelocityOutputMode> (outputModeIndex));
}

void SuperVelocityCurveAudioProcessor::applyProfileToEngine()
{
    profileStore.applyActiveToEngine (engine);
}

void SuperVelocityCurveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    engine.processMidiBuffer (midiMessages, buffer.getNumSamples());
}

juce::AudioProcessorEditor* SuperVelocityCurveAudioProcessor::createEditor()
{
    return new SuperVelocityCurveAudioProcessorEditor (*this);
}

void SuperVelocityCurveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state ("SuperVelocityCurveState");
    state.setProperty ("version", 2, nullptr);
    state.appendChild (profileStore.toValueTree(), nullptr);
    state.appendChild (apvts.copyState(), nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void SuperVelocityCurveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        const auto state = juce::ValueTree::fromXml (*xml);
        if (state.hasType ("SuperVelocityCurveState"))
        {
            for (int i = 0; i < state.getNumChildren(); ++i)
            {
                const auto child = state.getChild (i);
                if (child.hasType ("SuperVelocityCurveProfileStore")
                    || child.hasType ("SuperVelocityCurveProfile"))
                    profileStore.fromValueTree (child);
                else if (child.hasType ("Parameters"))
                    apvts.replaceState (child);
            }

            syncOutputModeToEngine();
            applyProfileToEngine();
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuperVelocityCurveAudioProcessor();
}
