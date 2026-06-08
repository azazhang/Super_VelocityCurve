#include "PadInspectorComponent.h"

PadInspectorComponent::PadInspectorComponent()
{
    addAndMakeVisible (enabledToggle);
    enabledToggle.onClick = [this] { currentPad.enabled = enabledToggle.getToggleState(); notifyChanged(); };

    for (auto* label : { &gateLabel, &retriggerLabel, &floorLabel, &ceilingLabel })
        addAndMakeVisible (label);

    setupSlider (velocityGateSlider, "");
    setupSlider (retriggerSlider, " ms");
    setupSlider (floorSlider, "");
    setupSlider (ceilingSlider, "");

    velocityGateSlider.setRange (0.0, 1.0, 0.01);
    retriggerSlider.setRange (0.0, 100.0, 1.0);
    floorSlider.setRange (0.0, 1.0, 0.01);
    ceilingSlider.setRange (0.0, 1.0, 0.01);

    velocityGateSlider.onValueChange = [this]
    {
        currentPad.velocityGate = static_cast<float> (velocityGateSlider.getValue());
        notifyChanged();
    };

    retriggerSlider.onValueChange = [this]
    {
        currentPad.retriggerGuardMs = retriggerSlider.getValue();
        notifyChanged();
    };

    floorSlider.onValueChange = [this]
    {
        currentPad.curve.setFloor (static_cast<float> (floorSlider.getValue()));
        notifyChanged();
    };

    ceilingSlider.onValueChange = [this]
    {
        currentPad.curve.setCeiling (static_cast<float> (ceilingSlider.getValue()));
        notifyChanged();
    };

    for (auto* slider : { &velocityGateSlider, &retriggerSlider, &floorSlider, &ceilingSlider })
        addAndMakeVisible (slider);
}

void PadInspectorComponent::setupSlider (juce::Slider& slider, const juce::String& suffix)
{
    slider.setTextValueSuffix (suffix);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
}

void PadInspectorComponent::setPad (const svc::ProfilePad& pad, int padIndex)
{
    currentPad = pad;
    currentPadIndex = padIndex;

    enabledToggle.setToggleState (pad.enabled, juce::dontSendNotification);
    velocityGateSlider.setValue (pad.velocityGate, juce::dontSendNotification);
    retriggerSlider.setValue (pad.retriggerGuardMs, juce::dontSendNotification);
    floorSlider.setValue (pad.curve.getFloor(), juce::dontSendNotification);
    ceilingSlider.setValue (pad.curve.getCeiling(), juce::dontSendNotification);
}

void PadInspectorComponent::notifyChanged()
{
    if (onPadChanged)
        onPadChanged (currentPadIndex, currentPad);
}

void PadInspectorComponent::paint (juce::Graphics& g)
{
    svc::ui::Theme::fillPanel (g, getLocalBounds().toFloat(), 10.0f);
    g.setColour (juce::Colour (svc::ui::Theme::textPrimary));
    g.setFont (svc::ui::Theme::sectionFont());
    g.drawText ("Pad Settings", getLocalBounds().removeFromTop (28).reduced (12, 0), juce::Justification::centredLeft);
}

void PadInspectorComponent::resized()
{
    auto area = getLocalBounds().reduced (12).withTrimmedTop (28);
    enabledToggle.setBounds (area.removeFromTop (24));
    area.removeFromTop (6);

    auto row = [&area] (juce::Label& label, juce::Slider& slider)
    {
        auto r = area.removeFromTop (58);
        label.setBounds (r.removeFromTop (16));
        slider.setBounds (r);
        area.removeFromTop (4);
    };

    row (gateLabel, velocityGateSlider);
    row (retriggerLabel, retriggerSlider);
    row (floorLabel, floorSlider);
    row (ceilingLabel, ceilingSlider);
}
