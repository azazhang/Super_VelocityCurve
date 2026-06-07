#pragma once

#include "../Profiles/ControllerProfile.h"
#include <JuceHeader.h>
#include <functional>
#include <vector>

class CurveEditorComponent : public juce::Component
{
public:
    CurveEditorComponent (const svc::ProfilePad& pad);

    void setPad (const svc::ProfilePad& pad);
    void addHitMarker (float inputNormalized, float outputNormalized);

    std::function<void (const svc::VelocityCurve&)> onCurveChanged;

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

private:
    svc::ProfilePad currentPad;
    svc::VelocityCurve workingCurve;
    std::vector<juce::Point<float>> hitMarkers;
    int draggedPointIndex = -1;

    juce::Point<float> normalizedToPoint (float input, float output) const;
    juce::Point<float> eventToNormalized (juce::Point<float> pos) const;
    int findNearestControlPoint (juce::Point<float> pos) const;
    void notifyCurveChanged();
};
