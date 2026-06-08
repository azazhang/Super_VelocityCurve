#include "VelocityCurve.h"
#include <algorithm>
#include <cmath>

namespace svc
{

VelocityCurve::VelocityCurve()
{
    setIdentity();
}

void VelocityCurve::setIdentity()
{
    controlPoints = makePresetPoints (CurvePreset::linear);
    rebuildLut();
}

void VelocityCurve::applyPreset (CurvePreset preset)
{
    controlPoints = makePresetPoints (preset);
    rebuildLut();
}

std::vector<CurveControlPoint> VelocityCurve::makePresetPoints (CurvePreset preset)
{
    switch (preset)
    {
        case CurvePreset::soft:
            return { { 0.0f, 0.0f }, { 0.25f, 0.45f }, { 0.5f, 0.68f }, { 0.75f, 0.85f }, { 1.0f, 1.0f } };
        case CurvePreset::hard:
            return { { 0.0f, 0.0f }, { 0.35f, 0.12f }, { 0.65f, 0.55f }, { 1.0f, 1.0f } };
        case CurvePreset::sCurve:
            return { { 0.0f, 0.0f }, { 0.2f, 0.08f }, { 0.5f, 0.5f }, { 0.8f, 0.92f }, { 1.0f, 1.0f } };
        case CurvePreset::exponential:
            return { { 0.0f, 0.0f }, { 0.15f, 0.03f }, { 0.4f, 0.18f }, { 0.7f, 0.52f }, { 1.0f, 1.0f } };
        case CurvePreset::logarithmic:
            return { { 0.0f, 0.0f }, { 0.3f, 0.48f }, { 0.6f, 0.78f }, { 0.85f, 0.93f }, { 1.0f, 1.0f } };
        case CurvePreset::linear:
        default:
            return { { 0.0f, 0.0f }, { 1.0f, 1.0f } };
    }
}

void VelocityCurve::setControlPoints (std::vector<CurveControlPoint> points)
{
    if (points.size() < 2)
    {
        setIdentity();
        return;
    }

    sortControlPoints (points);
    controlPoints = std::move (points);
    rebuildLut();
}

void VelocityCurve::sortControlPoints (std::vector<CurveControlPoint>& points)
{
    std::sort (points.begin(), points.end(), [] (const CurveControlPoint& a, const CurveControlPoint& b)
    {
        return a.input < b.input;
    });

    points.front().input = 0.0f;
    points.back().input = 1.0f;
}

void VelocityCurve::setFloor (float normalizedFloor) noexcept
{
    floor = std::clamp (normalizedFloor, 0.0f, 1.0f);
    if (floor > ceiling)
        ceiling = floor;
    rebuildLut();
}

void VelocityCurve::setCeiling (float normalizedCeiling) noexcept
{
    ceiling = std::clamp (normalizedCeiling, 0.0f, 1.0f);
    if (ceiling < floor)
        floor = ceiling;
    rebuildLut();
}

float VelocityCurve::interpolateControlPoints (const std::vector<CurveControlPoint>& points, float input)
{
    const auto clampedInput = std::clamp (input, 0.0f, 1.0f);

    if (points.empty())
        return clampedInput;

    if (clampedInput <= points.front().input)
        return points.front().output;

    if (clampedInput >= points.back().input)
        return points.back().output;

    for (size_t i = 1; i < points.size(); ++i)
    {
        const auto& a = points[i - 1];
        const auto& b = points[i];

        if (clampedInput <= b.input)
        {
            const auto span = b.input - a.input;
            if (span <= 0.0f)
                return b.output;

            const auto t = (clampedInput - a.input) / span;
            return a.output + t * (b.output - a.output);
        }
    }

    return points.back().output;
}

void VelocityCurve::rebuildLut()
{
    for (int i = 0; i < lutSize; ++i)
    {
        const auto input = static_cast<float> (i) / static_cast<float> (lutSize - 1);
        auto output = interpolateControlPoints (controlPoints, input);
        output = floor + output * (ceiling - floor);
        lut[static_cast<size_t> (i)] = static_cast<uint8_t> (normalizedToMidi1 (output));
    }
}

float VelocityCurve::mapNormalized (float input) const noexcept
{
    const auto clamped = std::clamp (input, 0.0f, 1.0f);
    const auto index = clamped * static_cast<float> (lutSize - 1);
    const auto i0 = static_cast<int> (index);
    const auto i1 = std::min (i0 + 1, lutSize - 1);
    const auto frac = index - static_cast<float> (i0);
    const auto v0 = midi1ToNormalized (lut[static_cast<size_t> (i0)]);
    const auto v1 = midi1ToNormalized (lut[static_cast<size_t> (i1)]);
    return v0 + frac * (v1 - v0);
}

int VelocityCurve::mapMidi1 (int input) const noexcept
{
    const auto clamped = std::clamp (input, 0, midi1Max);
    return static_cast<int> (lut[static_cast<size_t> (clamped)]);
}

int VelocityCurve::mapMidi2 (int input) const noexcept
{
    return normalizedToMidi2 (mapNormalized (midi2ToNormalized (input)));
}

} // namespace svc
