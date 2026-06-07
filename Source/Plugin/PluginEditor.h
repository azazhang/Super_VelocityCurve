#pragma once

#include "../UI/CurveEditorComponent.h"
#include "../UI/PadGridComponent.h"
#include "PluginProcessor.h"
#include <JuceHeader.h>

class SuperVelocityCurveAudioProcessorEditor : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit SuperVelocityCurveAudioProcessorEditor (SuperVelocityCurveAudioProcessor&);
    ~SuperVelocityCurveAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SuperVelocityCurveAudioProcessor& audioProcessor;

    juce::ComboBox profileBox;
    juce::ComboBox outputModeBox;
    juce::Slider retriggerSlider;
    juce::Label profileLabel { {}, "Controller Profile" };
    juce::Label outputModeLabel { {}, "Velocity Output" };
    juce::Label retriggerLabel { {}, "Retrigger Guard (ms)" };
    juce::Label titleLabel { {}, "Super VelocityCurve" };
    juce::Label hitFeedbackLabel;

    PadGridComponent padGrid;
    CurveEditorComponent curveEditor;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> profileAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> outputModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retriggerAttachment;

    int selectedPadIndex = 0;

    void timerCallback() override;
    void onPadSelected (int padIndex);
    void refreshSelectedPadCurve();
    void updateHitFeedback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperVelocityCurveAudioProcessorEditor)
};
