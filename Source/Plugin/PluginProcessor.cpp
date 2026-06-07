#include "PluginProcessor.h"
#include "PluginEditor.h"

SuperVelocityCurveAudioProcessor::SuperVelocityCurveAudioProcessor()
    : AudioProcessor (BusesProperties()),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    profileStore.getActiveProfile().applyToEngine (engine);

    apvts.addParameterListener ("profile", this);
    apvts.addParameterListener ("outputMode", this);
    apvts.addParameterListener ("retriggerGuard", this);
}

SuperVelocityCurveAudioProcessor::~SuperVelocityCurveAudioProcessor()
{
    apvts.removeParameterListener ("profile", this);
    apvts.removeParameterListener ("outputMode", this);
    apvts.removeParameterListener ("retriggerGuard", this);
}

void SuperVelocityCurveAudioProcessor::parameterChanged (const juce::String&, float)
{
    syncProfileToEngine();
}

juce::AudioProcessorValueTreeState::ParameterLayout SuperVelocityCurveAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "profile",
        "Controller Profile",
        juce::StringArray { "GM Standard", "Launchpad Drum Rack", "Maschine Group", "Roland SPD-SX", "Yamaha FGDP" },
        0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "outputMode",
        "Velocity Output",
        juce::StringArray { "Auto", "MIDI 1.0", "MIDI 2.0" },
        0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "retriggerGuard",
        "Retrigger Guard (ms)",
        juce::NormalisableRange<float> (0.0f, 100.0f, 1.0f),
        0.0f));

    return { params.begin(), params.end() };
}

void SuperVelocityCurveAudioProcessor::prepareToPlay (double, int)
{
    syncProfileToEngine();
}

void SuperVelocityCurveAudioProcessor::releaseResources()
{
}

void SuperVelocityCurveAudioProcessor::syncProfileToEngine()
{
    const auto profileIndex = static_cast<int> (*apvts.getRawParameterValue ("profile"));
    profileStore.setActiveProfileIndex (profileIndex);
    profileStore.getActiveProfile().applyToEngine (engine);

    const auto outputModeIndex = static_cast<int> (*apvts.getRawParameterValue ("outputMode"));
    engine.setOutputMode (static_cast<svc::VelocityOutputMode> (outputModeIndex));
    engine.setRetriggerGuardMs (static_cast<double> (*apvts.getRawParameterValue ("retriggerGuard")));
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
    state.setProperty ("version", 1, nullptr);
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
                if (child.hasType ("SuperVelocityCurveProfile"))
                    profileStore.fromValueTree (child);
                else if (child.hasType ("Parameters"))
                    apvts.replaceState (child);
            }

            syncProfileToEngine();
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SuperVelocityCurveAudioProcessor();
}
