#pragma once

#include "../Profiles/ControllerProfile.h"
#include "Theme.h"
#include <JuceHeader.h>
#include <functional>
#include <unordered_map>

class PadGridComponent : public juce::Component
{
public:
    PadGridComponent();

    void setProfile (const svc::ControllerProfile& profile);
    void setSelectedPadIndex (int index);
    int getSelectedPadIndex() const noexcept { return selectedPadIndex; }

    void flashPadHit (int note, int channel, float outputVelocity);
    void decayHitVisuals();

    std::function<void (int padIndex)> onPadSelected;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseMove (const juce::MouseEvent& event) override;

private:
    svc::ControllerProfile currentProfile;
    int selectedPadIndex = 0;
    int hoveredPadIndex = -1;

    struct HitVisual
    {
        float intensity = 0.0f;
        double lastUpdateMs = 0.0;
    };

    std::unordered_map<int, HitVisual> hitByPadIndex;

    juce::Viewport viewport;
    juce::Component padCanvas;

    int padIndexAt (juce::Point<int> pos) const;
    juce::Rectangle<int> padBoundsForIndex (int index) const;
    void updateCanvasSize();
    int cellWidth() const;
    int cellHeight() const;
};
