#include "CurveEditorComponent.h"

CurveEditorComponent::CurveEditorComponent() = default;

void CurveEditorComponent::setPad (const svc::ProfilePad& pad)
{
    currentPad = pad;
    hitMarkers.clear();
    repaint();
}

void CurveEditorComponent::addHitMarker (float inputNormalized, float outputNormalized)
{
    hitMarkers.push_back ({ inputNormalized, outputNormalized, juce::Time::getMillisecondCounterHiRes() });
    if (hitMarkers.size() > 48)
        hitMarkers.erase (hitMarkers.begin());
    repaint();
}

juce::Rectangle<float> CurveEditorComponent::plotArea() const
{
    return getLocalBounds().reduced (40, 36).withTrimmedTop (8).withTrimmedBottom (24).toFloat();
}

juce::Point<float> CurveEditorComponent::normalizedToPoint (float input, float output) const
{
    const auto plot = plotArea();
    return { plot.getX() + input * plot.getWidth(),
             plot.getBottom() - output * plot.getHeight() };
}

juce::Point<float> CurveEditorComponent::eventToNormalized (juce::Point<float> pos) const
{
    const auto plot = plotArea();
    const auto input = juce::jlimit (0.0f, 1.0f, (pos.x - plot.getX()) / plot.getWidth());
    const auto output = juce::jlimit (0.0f, 1.0f, 1.0f - (pos.y - plot.getY()) / plot.getHeight());
    return { input, output };
}

int CurveEditorComponent::findNearestControlPoint (juce::Point<float> pos) const
{
    const auto& points = currentPad.curve.getControlPoints();
    int nearest = -1;
    float bestDistance = 14.0f;

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

void CurveEditorComponent::notifyChanged()
{
    if (onPadChanged)
        onPadChanged (currentPad);
}

void CurveEditorComponent::applyPreset (svc::CurvePreset preset)
{
    currentPad.curve.applyPreset (preset);
    notifyChanged();
    repaint();
}

void CurveEditorComponent::resetCurve()
{
    currentPad.curve.setIdentity();
    notifyChanged();
    repaint();
}

void CurveEditorComponent::copyFrom (const svc::VelocityCurve& other)
{
    currentPad.curve.setControlPoints (other.getControlPoints());
    currentPad.curve.setFloor (other.getFloor());
    currentPad.curve.setCeiling (other.getCeiling());
    notifyChanged();
    repaint();
}

void CurveEditorComponent::setFloorCeiling (float floor, float ceiling)
{
    currentPad.curve.setFloor (floor);
    currentPad.curve.setCeiling (ceiling);
    notifyChanged();
    repaint();
}

void CurveEditorComponent::drawGrid (juce::Graphics& g) const
{
    const auto plot = plotArea();
    g.setColour (juce::Colour (svc::ui::Theme::curveGrid));

    for (int i = 0; i <= 4; ++i)
    {
        const auto t = static_cast<float> (i) / 4.0f;
        const auto x = plot.getX() + t * plot.getWidth();
        const auto y = plot.getBottom() - t * plot.getHeight();
        g.drawVerticalLine (static_cast<int> (x), plot.getY(), plot.getBottom());
        g.drawHorizontalLine (static_cast<int> (y), plot.getX(), plot.getRight());
    }

    g.setColour (juce::Colour (svc::ui::Theme::textSecondary));
    g.setFont (svc::ui::Theme::smallFont());
    g.drawText ("0", static_cast<int> (plot.getX()) - 18, static_cast<int> (plot.getBottom()) - 6, 16, 12, juce::Justification::centred);
    g.drawText ("127", static_cast<int> (plot.getRight()) - 8, static_cast<int> (plot.getY()) - 14, 28, 12, juce::Justification::centred);
    g.drawText ("Input", static_cast<int> (plot.getCentreX()) - 20, static_cast<int> (plot.getBottom()) + 6, 40, 14, juce::Justification::centred);
    g.drawText ("Out", static_cast<int> (plot.getX()) - 34, static_cast<int> (plot.getCentreY()) - 7, 28, 14, juce::Justification::centred);

    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawLine (plot.getX(), plot.getBottom(), plot.getRight(), plot.getY(), 1.0f);
}

void CurveEditorComponent::drawCurve (juce::Graphics& g) const
{
    const auto plot = plotArea();
    juce::Path curvePath;
    const auto& lut = currentPad.curve.getLut();

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

    g.setColour (juce::Colour (svc::ui::Theme::curveLine).withAlpha (0.25f));
    g.strokePath (curvePath, juce::PathStrokeType (6.0f));
    g.setColour (juce::Colour (svc::ui::Theme::curveLine));
    g.strokePath (curvePath, juce::PathStrokeType (2.5f));

    const auto now = juce::Time::getMillisecondCounterHiRes();
    for (const auto& hit : hitMarkers)
    {
        const auto age = now - hit.createdMs;
        const auto alpha = juce::jlimit (0.0f, 1.0f, 1.0f - static_cast<float> (age / 1200.0));
        const auto p = normalizedToPoint (hit.input, hit.output);
        g.setColour (juce::Colour (svc::ui::Theme::curveHit).withAlpha (alpha));
        g.fillEllipse (p.x - 4.0f, p.y - 4.0f, 8.0f, 8.0f);
    }

    const auto& points = currentPad.curve.getControlPoints();
    for (int i = 0; i < static_cast<int> (points.size()); ++i)
    {
        const auto& point = points[static_cast<size_t> (i)];
        const auto p = normalizedToPoint (point.input, point.output);
        const auto isEndpoint = i == 0 || i == static_cast<int> (points.size()) - 1;
        g.setColour (isEndpoint ? juce::Colour (svc::ui::Theme::accentWarm)
                                : juce::Colours::white);
        g.fillEllipse (p.x - (isEndpoint ? 6.0f : 5.0f), p.y - (isEndpoint ? 6.0f : 5.0f),
                       isEndpoint ? 12.0f : 10.0f, isEndpoint ? 12.0f : 10.0f);
        g.setColour (juce::Colour (svc::ui::Theme::border));
        g.drawEllipse (p.x - 5.0f, p.y - 5.0f, 10.0f, 10.0f, 1.0f);
    }

    juce::ignoreUnused (plot);
}

void CurveEditorComponent::paint (juce::Graphics& g)
{
    svc::ui::Theme::fillPanel (g, getLocalBounds().toFloat(), 10.0f);

    g.setColour (juce::Colour (svc::ui::Theme::textPrimary));
    g.setFont (svc::ui::Theme::sectionFont());
    g.drawText ("Velocity Curve — " + currentPad.label,
                getLocalBounds().removeFromTop (28).reduced (12, 0),
                juce::Justification::centredLeft);

    const auto plot = plotArea();
    g.setColour (juce::Colour (svc::ui::Theme::background));
    g.fillRoundedRectangle (plot, 6.0f);

    drawGrid (g);
    drawCurve (g);
}

void CurveEditorComponent::mouseDown (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        const auto index = findNearestControlPoint (event.position);
        auto points = currentPad.curve.getControlPoints();
        if (index > 0 && index < static_cast<int> (points.size()) - 1)
        {
            points.erase (points.begin() + index);
            currentPad.curve.setControlPoints (points);
            notifyChanged();
            repaint();
        }
        return;
    }

    draggedPointIndex = findNearestControlPoint (event.position);
}

void CurveEditorComponent::mouseDrag (const juce::MouseEvent& event)
{
    if (draggedPointIndex < 0)
        return;

    auto points = currentPad.curve.getControlPoints();
    if (draggedPointIndex >= static_cast<int> (points.size()))
        return;

    const auto normalized = eventToNormalized (event.position);
    auto& point = points[static_cast<size_t> (draggedPointIndex)];

    if (draggedPointIndex == 0)
        point.input = 0.0f;
    else if (draggedPointIndex == static_cast<int> (points.size()) - 1)
        point.input = 1.0f;
    else
        point.input = juce::jlimit (points[static_cast<size_t> (draggedPointIndex - 1)].input + 0.02f,
                                    points[static_cast<size_t> (draggedPointIndex + 1)].input - 0.02f,
                                    normalized.x);

    point.output = normalized.y;
    currentPad.curve.setControlPoints (points);
    notifyChanged();
    repaint();
}

void CurveEditorComponent::mouseUp (const juce::MouseEvent&)
{
    draggedPointIndex = -1;
}

void CurveEditorComponent::mouseDoubleClick (const juce::MouseEvent& event)
{
    if (findNearestControlPoint (event.position) >= 0)
        return;

    auto points = currentPad.curve.getControlPoints();
    const auto normalized = eventToNormalized (event.position);
    points.push_back ({ normalized.x, normalized.y });
    currentPad.curve.setControlPoints (points);
    notifyChanged();
    repaint();
}
