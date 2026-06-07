#pragma once

#include "MidiVelocity.h"
#include <array>
#include <vector>

namespace svc
{

struct CurveControlPoint
{
    float input = 0.0f;
    float output = 0.0f;
};

// Per-pad velocity curve stored as control points and a 128-entry runtime LUT.
// High-resolution MIDI 2.0 input is mapped via normalized float + LUT interpolation.
class VelocityCurve
{
public:
    static constexpr int lutSize = 128;

    VelocityCurve();

    void setIdentity();
    void setControlPoints (const std::vector<CurveControlPoint>& points);
    const std::vector<CurveControlPoint>& getControlPoints() const noexcept { return controlPoints; }

    void setFloor (float normalizedFloor) noexcept;
    void setCeiling (float normalizedCeiling) noexcept;
    float getFloor() const noexcept { return floor; }
    float getCeiling() const noexcept { return ceiling; }

    void rebuildLut();
    float mapNormalized (float input) const noexcept;
    int mapMidi1 (int input) const noexcept;
    int mapMidi2 (int input) const noexcept;

    const std::array<uint8_t, lutSize>& getLut() const noexcept { return lut; }

private:
    std::vector<CurveControlPoint> controlPoints;
    std::array<uint8_t, lutSize> lut {};
    float floor = 0.0f;
    float ceiling = 1.0f;

    static float interpolateControlPoints (const std::vector<CurveControlPoint>& points, float input);
};

} // namespace svc
