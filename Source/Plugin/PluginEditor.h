#pragma once

#include "../UI/CurveEditorComponent.h"
#include "../UI/PadGridComponent.h"
#include "../UI/PadInspectorComponent.h"
#include "../UI/Theme.h"
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

    juce::Label titleLabel { {}, "Super VelocityCurve" };
    juce::Label subtitleLabel { {}, "Per-pad velocity curves for finger drummers" };
    juce::ComboBox profileBox;
    juce::ComboBox outputModeBox;
    juce::ComboBox curvePresetBox;
    juce::TextEditor profileNameEditor;
    juce::TextButton saveProfileButton { "Save Profile" };
    juce::TextButton duplicateProfileButton { "Duplicate" };
    juce::TextButton deleteProfileButton { "Delete" };
    juce::TextButton importButton { "Import" };
    juce::TextButton exportButton { "Export" };
    juce::TextButton resetCurveButton { "Reset Curve" };
    juce::Label profileLabel { {}, "Profile" };
    juce::Label outputModeLabel { {}, "MIDI output" };
    juce::Label presetLabel { {}, "Curve preset" };
    juce::Label liveHitsLabel { {}, "Live" };

    PadGridComponent padGrid;
    CurveEditorComponent curveEditor;
    PadInspectorComponent padInspector;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> outputModeAttachment;

    int selectedPadIndex = 0;

    void timerCallback() override;
    void rebuildProfileList();
    void onProfileSelected();
    void onPadSelected (int padIndex);
    void refreshPadUI();
    void updateSelectedPadFromUI (const svc::ProfilePad& pad);
    void applyProfileToEngine();
    void updateLiveHits();
    void showStatus (const juce::String& message, bool isError = false);

    juce::String statusMessage;
    bool statusIsError = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SuperVelocityCurveAudioProcessorEditor)
};
