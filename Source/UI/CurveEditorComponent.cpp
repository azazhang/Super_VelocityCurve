#include "CurveEditorComponent.h"

CurveEditorComponent::CurveEditorComponent (const svc::ProfilePad& pad)
    : currentPad (pad), workingCurve (pad.curve)
{
}

void CurveEditorComponent::setPad (const svc::ProfilePad& pad)
{
    currentPad = pad;
    workingCurve = pad.curve;
    hitMarkers.clear();
    repaint();
}

void CurveEditorComponent::addHitMarker (float inputNormalized, float outputNormalized)
{
    hitMarkers.push_back ({ inputNormalized, outputNormalized });
    if (hitMarkers.size() > 64)
        hitMarkers.erase (hitMarkers.begin());
}

juce::Point<float> CurveEditorComponent::normalizedToPoint (float input, float output) const
{
    const auto plot = getLocalBounds().reduced (28, 20).toFloat();
    return { plot.getX() + input * plot.getWidth(),
             plot.getBottom() - output * plot.getHeight() };
}

juce::Point<float> CurveEditorComponent::eventToNormalized (juce::Point<float> pos) const
{
    const auto plot = getLocalBounds().reduced (28, 20).toFloat();
    const auto input = juce::jlimit (0.0f, 1.0f, (pos.x - plot.getX()) / plot.getWidth());
    const auto output = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - plot.getY()) / plot.getHeight());
    return { input, output };
}

int CurveEditorComponent::findNearestControlPoint (juce::Point<float> pos) const
{
    const auto& points = workingCurve.getControlPoints();
    int nearest = -1;
    float bestDistance = 12.0f;

    for (int i = 0; i < static_cast<int> (points.size()); ++i)
    {
        const auto screen = normalizedToPoint (points[static_cast<size_t> (i)].input,
                                               points[static_cast<size_t> (i)].output);
        const auto distance = screen.getDistanceFrom (pos);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            nearest = i;
        }
    }

    return nearest;
}

void CurveEditorComponent::notifyCurveChanged()
{
    if (onCurveChanged)
        onCurveChanged (workingCurve);
}

void CurveEditorComponent::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff252a33));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("Velocity Curve — " + currentPad.label,
                getLocalBounds().removeFromTop (22),
                juce::Justification::centred);

    const auto plot = getLocalBounds().reduced (28, 20);

    g.setColour (juce::Colour (0xff1e2229));
    g.fillRect (plot);

    g.setColour (juce::Colours::white.withAlpha (0.15f));
    for (int i = 1; i < 4; ++i)
    {
        const auto x = plot.getX() + (plot.getWidth() * i) / 4;
        const auto y = plot.getY() + (plot.getHeight() * i) / 4;
        g.drawVerticalLine (x, static_cast<float> (plot.getY()), static_cast<float> (plot.getBottom()));
        g.drawHorizontalLine (y, static_cast<float> (plot.getX()), static_cast<float> (plot.getRight()));
    }

    juce::Path curvePath;
    const auto& lut = workingCurve.getLut();
    for (int i = 0; i < svc::VelocityCurve::lutSize; ++i)
    {
        const auto input = static_cast<float> (i) / static_cast<float> (svc::VelocityCurve::lutSize - 1);
        const auto output = static_cast<float> (lut[static_cast<size_t> (i)]) / 127.0f;
        const auto point = normalizedToPoint (input, output);

        if (i == 0)
            curvePath.startNewSubPath (point);
        else
            curvePath.lineTo (point);
    }

    g.setColour (juce::Colour (0xff4a9eff));
    g.strokePath (curvePath, juce::PathStrokeType (2.0f));

    g.setColour (juce::Colours::orange.withAlpha (0.8f));
    for (const auto& hit : hitMarkers)
    {
        const auto p = normalizedToPoint (hit.x, hit.y);
        g.fillEllipse (p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f);
    }

    const auto& points = workingCurve.getControlPoints();
    for (const auto& point : points)
    {
        const auto p = normalizedToPoint (point.input, point.output);
        g.setColour (juce::Colours::white);
        g.fillEllipse (p.x - 5.0f, p.y - 5.0f, 10.0f, 10.0f);
    }

    g.setColour (juce::Colours::lightgrey);
    g.setFont (10.0f);
    g.drawText ("In", plot.getX() - 20, plot.getBottom() + 2, 40, 14, juce::Justification::centred);
    g.drawText ("Out", plot.getX() - 26, plot.getY() - 8, 40, 14, juce::Justification::centred);
}

void CurveEditorComponent::mouseDown (const juce::MouseEvent& event)
{
    draggedPointIndex = findNearestControlPoint (event.position);
}

void CurveEditorComponent::mouseDrag (const juce::MouseEvent& event)
{
    if (draggedPointIndex < 0)
        return;

    auto points = workingCurve.getControlPoints();
    if (draggedPointIndex >= static_cast<int> (points.size()))
        return;

    const auto normalized = eventToNormalized (event.position);
    auto& point = points[static_cast<size_t> (draggedPointIndex)];

    if (draggedPointIndex == 0)
        point.input = 0.0f;
    else if (draggedPointIndex == static_cast<int> (points.size()) - 1)
        point.input = 1.0f;
    else
        point.input = juce::jlimit (points[static_cast<size_t> (draggedPointIndex - 1)].input + 0.01f,
                                  points[static_cast<size_t> (draggedPointIndex + 1)].input - 0.01f,
                                  normalized.x);

    point.output = normalized.y;
    workingCurve.setControlPoints (points);
    notifyCurveChanged();
    repaint();
}

void CurveEditorComponent::mouseUp (const juce::MouseEvent&)
{
    draggedPointIndex = -1;
}
