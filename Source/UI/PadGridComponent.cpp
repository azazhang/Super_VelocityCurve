#include "PadGridComponent.h"

PadGridComponent::PadGridComponent (const svc::ControllerProfile& profile)
    : currentProfile (profile)
{
}

void PadGridComponent::setProfile (const svc::ControllerProfile& profile)
{
    currentProfile = profile;
    selectedPadIndex = 0;
    repaint();
}

void PadGridComponent::setSelectedPadIndex (int index)
{
    selectedPadIndex = juce::jlimit (0, juce::jmax (0, static_cast<int> (currentProfile.getPads().size()) - 1), index);
    repaint();
}

juce::Rectangle<int> PadGridComponent::padBoundsForIndex (int index) const
{
    const auto& pads = currentProfile.getPads();
    if (index < 0 || index >= static_cast<int> (pads.size()))
        return {};

    const auto& pad = pads[static_cast<size_t> (index)];
    const auto area = getLocalBounds().reduced (8);
    const int cols = 6;
    const int rows = 4;
    const int cellW = area.getWidth() / cols;
    const int cellH = area.getHeight() / rows;

    return { area.getX() + pad.gridCol * cellW,
             area.getY() + pad.gridRow * cellH,
             cellW - 4,
             cellH - 4 };
}

void PadGridComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff252a33));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

    g.setColour (juce::Colours::white.withAlpha (0.8f));
    g.setFont (14.0f);
    g.drawText ("Pads", getLocalBounds().removeFromTop (22), juce::Justification::centred);

    const auto& pads = currentProfile.getPads();
    for (int i = 0; i < static_cast<int> (pads.size()); ++i)
    {
        auto bounds = padBoundsForIndex (i);
        const auto& pad = pads[static_cast<size_t> (i)];

        const auto isSelected = i == selectedPadIndex;
        g.setColour (isSelected ? juce::Colour (0xff4a9eff) : juce::Colour (0xff3a4250));
        g.fillRoundedRectangle (bounds.toFloat(), 4.0f);

        g.setColour (juce::Colours::white);
        g.setFont (11.0f);
        auto labelArea = bounds.removeFromTop (bounds.getHeight() / 2);
        g.drawFittedText (pad.label, labelArea, juce::Justification::centred, 2);
        g.setFont (10.0f);
        g.setColour (juce::Colours::lightgrey);
        g.drawText ("N" + juce::String (pad.midiNote) + " Ch" + juce::String (pad.midiChannel),
                    bounds,
                    juce::Justification::centred);
    }
}

void PadGridComponent::resized()
{
}

void PadGridComponent::mouseDown (const juce::MouseEvent& event)
{
    const auto& pads = currentProfile.getPads();
    for (int i = 0; i < static_cast<int> (pads.size()); ++i)
    {
        if (padBoundsForIndex (i).contains (event.getPosition()))
        {
            setSelectedPadIndex (i);
            if (onPadSelected)
                onPadSelected (i);
            return;
        }
    }
}
