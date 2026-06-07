#pragma once

#include "../Engine/VelocityEngine.h"
#include "../Profiles/ProfileStore.h"
#include <JuceHeader.h>

class SuperVelocityCurveAudioProcessor : public juce::AudioProcessor,
                                         private juce::AudioProcessorValueTreeState::Listener
{
public:
    SuperVelocityCurveAudioProcessor();
    ~SuperVelocityCurveAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    svc::VelocityEngine& getEngine() noexcept { return engine; }
    svc::ProfileStore& getProfileStore() noexcept { return profileStore; }

    juce::AudioProcessorValueTreeState& getApvts() noexcept { return apvts; }

private:
    svc::VelocityEngine engine;
    svc::ProfileStore profileStore;
    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void syncProfileToEngine();
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperVelocityCurveAudioProcessor)
};
