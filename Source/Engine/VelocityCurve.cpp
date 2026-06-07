#include "VelocityCurve.h"
#include <algorithm>

namespace svc
{

VelocityCurve::VelocityCurve()
{
    setIdentity();
}

void VelocityCurve::setIdentity()
{
    controlPoints = {
        { 0.0f, 0.0f },
        { 1.0f, 1.0f }
    };
    rebuildLut();
}

void VelocityCurve::setControlPoints (const std::vector<CurveControlPoint>& points)
{
    controlPoints = points;
    if (controlPoints.size() < 2)
        setIdentity();
    else
        rebuildLut();
}

void VelocityCurve::setFloor (float normalizedFloor) noexcept
{
    floor = std::clamp (normalizedFloor, 0.0f, 1.0f);
    rebuildLut();
}

void VelocityCurve::setCeiling (float normalizedCeiling) noexcept
{
    ceiling = std::clamp (normalizedCeiling, 0.0f, 1.0f);
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
