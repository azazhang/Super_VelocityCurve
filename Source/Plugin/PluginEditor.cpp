#include "PluginEditor.h"

SuperVelocityCurveAudioProcessorEditor::SuperVelocityCurveAudioProcessorEditor (SuperVelocityCurveAudioProcessor& p)
    : AudioProcessorEditor (p),
      audioProcessor (p),
      padGrid (p.getProfileStore().getActiveProfile()),
      curveEditor (p.getProfileStore().getActiveProfile().getPads().empty()
                       ? svc::ProfilePad {}
                       : p.getProfileStore().getActiveProfile().getPads().front())
{
    setSize (960, 620);

    titleLabel.setFont (juce::Font (22.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    profileBox.addItemList ({ "GM Standard", "Launchpad Drum Rack", "Maschine Group", "Roland SPD-SX", "Yamaha FGDP" }, 1);
    addAndMakeVisible (profileBox);
    addAndMakeVisible (profileLabel);

    outputModeBox.addItemList ({ "Auto (match input)", "MIDI 1.0 (7-bit)", "MIDI 2.0 (high-res)" }, 1);
    addAndMakeVisible (outputModeBox);
    addAndMakeVisible (outputModeLabel);

    retriggerSlider.setRange (0.0, 100.0, 1.0);
    retriggerSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible (retriggerSlider);
    addAndMakeVisible (retriggerLabel);

    hitFeedbackLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (hitFeedbackLabel);

    addAndMakeVisible (padGrid);
    addAndMakeVisible (curveEditor);

    profileAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.getApvts(), "profile", profileBox);
    outputModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.getApvts(), "outputMode", outputModeBox);
    retriggerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.getApvts(), "retriggerGuard", retriggerSlider);

    padGrid.onPadSelected = [this] (int index) { onPadSelected (index); };

    curveEditor.onCurveChanged = [this] (const svc::VelocityCurve& curve)
    {
        auto& profile = audioProcessor.getProfileStore().getActiveProfile();
        auto& pads = profile.getPads();
        if (selectedPadIndex >= 0 && selectedPadIndex < static_cast<int> (pads.size()))
        {
            pads[static_cast<size_t> (selectedPadIndex)].curve = curve;
            profile.applyToEngine (audioProcessor.getEngine());
        }
    };

    profileBox.onChange = [this]
    {
        const auto index = profileBox.getSelectedItemIndex();
        audioProcessor.getProfileStore().setActiveProfileIndex (index);
        padGrid.setProfile (audioProcessor.getProfileStore().getActiveProfile());
        selectedPadIndex = 0;
        padGrid.setSelectedPadIndex (0);
        refreshSelectedPadCurve();
    };

    onPadSelected (0);
    startTimerHz (30);
}

void SuperVelocityCurveAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1d24));
    g.setColour (juce::Colour (0xff2d3340));
    g.fillRoundedRectangle (getLocalBounds().reduced (8).toFloat(), 8.0f);
}

void SuperVelocityCurveAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    auto header = bounds.removeFromTop (36);
    titleLabel.setBounds (header);

    auto controls = bounds.removeFromTop (72);
    auto col = controls.removeFromLeft (controls.getWidth() / 3).reduced (4);
    profileLabel.setBounds (col.removeFromTop (18));
    profileBox.setBounds (col);

    col = controls.removeFromLeft (controls.getWidth() / 2).reduced (4);
    outputModeLabel.setBounds (col.removeFromTop (18));
    outputModeBox.setBounds (col);

    col = controls.reduced (4);
    retriggerLabel.setBounds (col.removeFromTop (18));
    retriggerSlider.setBounds (col);

    auto footer = bounds.removeFromBottom (24);
    hitFeedbackLabel.setBounds (footer);

    auto left = bounds.removeFromLeft (bounds.getWidth() / 2).reduced (4);
    padGrid.setBounds (left);
    curveEditor.setBounds (bounds.reduced (4));
}

void SuperVelocityCurveAudioProcessorEditor::onPadSelected (int padIndex)
{
    selectedPadIndex = padIndex;
    refreshSelectedPadCurve();
}

void SuperVelocityCurveAudioProcessorEditor::refreshSelectedPadCurve()
{
    const auto& pads = audioProcessor.getProfileStore().getActiveProfile().getPads();
    if (selectedPadIndex >= 0 && selectedPadIndex < static_cast<int> (pads.size()))
        curveEditor.setPad (pads[static_cast<size_t> (selectedPadIndex)]);
}

void SuperVelocityCurveAudioProcessorEditor::updateHitFeedback()
{
    svc::HitEvent hit;
    juce::String text = "Live hits: ";
    int count = 0;

    while (audioProcessor.getEngine().getHitFifo().pop (hit) && count < 5)
    {
        const auto inVel = juce::String (static_cast<int> (hit.inputVelocity * 127.0f));
        const auto outVel = juce::String (static_cast<int> (hit.outputVelocity * 127.0f));
        const auto protocol = hit.isMidi2 ? "MIDI2" : "MIDI1";
        text += "[" + juce::String (hit.note) + " " + protocol + " " + inVel + "->" + outVel + "] ";
        curveEditor.addHitMarker (hit.inputVelocity, hit.outputVelocity);
        ++count;
    }

    if (count == 0)
        text += "play your controller to see input/output velocity";

    hitFeedbackLabel.setText (text, juce::dontSendNotification);
}

void SuperVelocityCurveAudioProcessorEditor::timerCallback()
{
    updateHitFeedback();
    curveEditor.repaint();
}
