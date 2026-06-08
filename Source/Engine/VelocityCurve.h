#pragma once

#include "MidiVelocity.h"
#include <array>
#include <vector>

namespace svc
{

enum class CurvePreset
{
    linear,
    soft,
    hard,
    sCurve,
    exponential,
    logarithmic
};

struct CurveControlPoint
{
    float input = 0.0f;
    float output = 0.0f;
};

class VelocityCurve
{
public:
    static constexpr int lutSize = 128;

    VelocityCurve();

    void setIdentity();
    void applyPreset (CurvePreset preset);
    void setControlPoints (std::vector<CurveControlPoint> points);
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

    static std::vector<CurveControlPoint> makePresetPoints (CurvePreset preset);

private:
    std::vector<CurveControlPoint> controlPoints;
    std::array<uint8_t, lutSize> lut {};
    float floor = 0.0f;
    float ceiling = 1.0f;

    static void sortControlPoints (std::vector<CurveControlPoint>& points);
    static float interpolateControlPoints (const std::vector<CurveControlPoint>& points, float input);
};

} // namespace svc
