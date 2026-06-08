#pragma once

#include "../Profiles/ControllerProfile.h"
#include "Theme.h"
#include <JuceHeader.h>
#include <functional>
#include <vector>

class CurveEditorComponent : public juce::Component
{
public:
    CurveEditorComponent();

    void setPad (const svc::ProfilePad& pad);
    const svc::ProfilePad& getPad() const noexcept { return currentPad; }

    void addHitMarker (float inputNormalized, float outputNormalized);

    std::function<void (const svc::ProfilePad&)> onPadChanged;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;

    void applyPreset (svc::CurvePreset preset);
    void resetCurve();
    void copyFrom (const svc::VelocityCurve& other);
    void setFloorCeiling (float floor, float ceiling);

private:
    svc::ProfilePad currentPad;

    struct HitMarker
    {
        float input = 0.0f;
        float output = 0.0f;
        double createdMs = 0.0;
    };

    std::vector<HitMarker> hitMarkers;
    int draggedPointIndex = -1;

    juce::Rectangle<float> plotArea() const;
    juce::Point<float> normalizedToPoint (float input, float output) const;
    juce::Point<float> eventToNormalized (juce::Point<float> pos) const;
    int findNearestControlPoint (juce::Point<float> pos) const;
    void notifyChanged();
    void drawGrid (juce::Graphics& g) const;
    void drawCurve (juce::Graphics& g) const;
};
