#include "PadGridComponent.h"

PadGridComponent::PadGridComponent()
{
    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&padCanvas, false);
    viewport.setScrollBarsShown (true, true);
    padCanvas.setInterceptsMouseClicks (false, false);
}

void PadGridComponent::setProfile (const svc::ControllerProfile& profile)
{
    currentProfile = profile;
    selectedPadIndex = juce::jlimit (0, juce::jmax (0, static_cast<int> (profile.getPads().size()) - 1), selectedPadIndex);
    hitByPadIndex.clear();
    updateCanvasSize();
    repaint();
}

void PadGridComponent::setSelectedPadIndex (int index)
{
    selectedPadIndex = juce::jlimit (0, juce::jmax (0, static_cast<int> (currentProfile.getPads().size()) - 1), index);
    repaint();
}

void PadGridComponent::flashPadHit (int note, int channel, float outputVelocity)
{
    const auto& pads = currentProfile.getPads();
    for (int i = 0; i < static_cast<int> (pads.size()); ++i)
    {
        const auto& pad = pads[static_cast<size_t> (i)];
        if (pad.midiNote == note && pad.midiChannel == channel)
        {
            hitByPadIndex[i] = { outputVelocity, juce::Time::getMillisecondCounterHiRes() };
            repaint();
            return;
        }
    }
}

void PadGridComponent::decayHitVisuals()
{
    const auto now = juce::Time::getMillisecondCounterHiRes();
    bool changed = false;

    for (auto it = hitByPadIndex.begin(); it != hitByPadIndex.end();)
    {
        const auto age = now - it->second.lastUpdateMs;
        if (age > 500.0)
        {
            it = hitByPadIndex.erase (it);
            changed = true;
        }
        else
        {
            it->second.intensity = juce::jmax (0.0f, it->second.intensity - 0.08f);
            ++it;
            changed = true;
        }
    }

    if (changed)
        repaint();
}

int PadGridComponent::cellWidth() const
{
    return 88;
}

int PadGridComponent::cellHeight() const
{
    return 72;
}

void PadGridComponent::updateCanvasSize()
{
    int rows = 1;
    int cols = 1;
    currentProfile.getGridDimensions (rows, cols);

    padCanvas.setSize (juce::jmax (cols * cellWidth() + 16, getWidth()),
                       juce::jmax (rows * cellHeight() + 16, getHeight()));
}

juce::Rectangle<int> PadGridComponent::padBoundsForIndex (int index) const
{
    const auto& pads = currentProfile.getPads();
    if (index < 0 || index >= static_cast<int> (pads.size()))
        return {};

    const auto& pad = pads[static_cast<size_t> (index)];
    const int padW = cellWidth() - 6;
    const int padH = cellHeight() - 6;

    return { 8 + pad.gridCol * cellWidth(),
             8 + pad.gridRow * cellHeight(),
             padW,
             padH };
}

int PadGridComponent::padIndexAt (juce::Point<int> pos) const
{
    const auto canvasPos = pos + viewport.getViewPosition();
    const auto& pads = currentProfile.getPads();

    for (int i = static_cast<int> (pads.size()) - 1; i >= 0; --i)
    {
        if (padBoundsForIndex (i).contains (canvasPos))
            return i;
    }

    return -1;
}

void PadGridComponent::paint (juce::Graphics& g)
{
    svc::ui::Theme::fillPanel (g, getLocalBounds().toFloat(), 10.0f);

    g.setColour (juce::Colour (svc::ui::Theme::textPrimary));
    g.setFont (svc::ui::Theme::sectionFont());
    g.drawText ("Pad Layout", getLocalBounds().removeFromTop (28).reduced (12, 0), juce::Justification::centredLeft);

    const auto viewArea = getLocalBounds().reduced (8).withTrimmedTop (28);
    g.reduceClipRegion (viewArea);

    const auto origin = viewArea.getPosition() - viewport.getViewPosition();
    const auto& pads = currentProfile.getPads();

    for (int i = 0; i < static_cast<int> (pads.size()); ++i)
    {
        auto bounds = padBoundsForIndex (i).translated (origin.x, origin.y);
        const auto& pad = pads[static_cast<size_t> (i)];

        juce::uint32 baseColour = svc::ui::Theme::padIdle;
        if (! pad.enabled)
            baseColour = svc::ui::Theme::padDisabled;
        else if (i == selectedPadIndex)
            baseColour = svc::ui::Theme::padSelected;
        else if (i == hoveredPadIndex)
            baseColour = svc::ui::Theme::padHover;

        const auto hitIt = hitByPadIndex.find (i);
        if (hitIt != hitByPadIndex.end())
        {
            const auto blend = juce::jlimit (0.0f, 1.0f, hitIt->second.intensity);
            baseColour = juce::Colour (baseColour).interpolatedWith (juce::Colour (svc::ui::Theme::padHit), blend).getARGB();
        }

        g.setColour (juce::Colour (baseColour));
        g.fillRoundedRectangle (bounds.toFloat(), 6.0f);

        g.setColour (juce::Colour (svc::ui::Theme::border));
        g.drawRoundedRectangle (bounds.toFloat(), 6.0f, 1.0f);

        auto textArea = bounds.reduced (6);
        g.setColour (pad.enabled ? juce::Colour (svc::ui::Theme::textPrimary)
                                 : juce::Colour (svc::ui::Theme::textSecondary));
        g.setFont (svc::ui::Theme::bodyFont().boldened());
        g.drawFittedText (pad.label, textArea.removeFromTop (textArea.getHeight() / 2), juce::Justification::centred, 2);

        g.setFont (svc::ui::Theme::smallFont());
        g.setColour (juce::Colour (svc::ui::Theme::textSecondary));
        g.drawText ("N" + juce::String (pad.midiNote) + " · Ch" + juce::String (pad.midiChannel),
                    textArea,
                    juce::Justification::centred);

        if (pad.retriggerGuardMs > 0.0)
        {
            g.setColour (juce::Colour (svc::ui::Theme::accentWarm).withAlpha (0.85f));
            g.fillEllipse (static_cast<float> (bounds.getRight()) - 12.0f,
                           static_cast<float> (bounds.getY()) + 4.0f,
                           8.0f,
                           8.0f);
        }
    }
}

void PadGridComponent::resized()
{
    auto area = getLocalBounds().reduced (8).withTrimmedTop (28);
    viewport.setBounds (area);
    updateCanvasSize();
}

void PadGridComponent::mouseDown (const juce::MouseEvent& event)
{
    const auto index = padIndexAt (event.getPosition());
    if (index >= 0)
    {
        setSelectedPadIndex (index);
        if (onPadSelected)
            onPadSelected (index);
    }
}

void PadGridComponent::mouseMove (const juce::MouseEvent& event)
{
    const auto index = padIndexAt (event.getPosition());
    if (index != hoveredPadIndex)
    {
        hoveredPadIndex = index;
        repaint();
    }
}
