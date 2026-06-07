#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace svc
{

// MIDI 1.0 and MIDI 2.0 velocity helpers. Curves are authored in normalized space
// so profiles remain valid as hosts and controllers adopt higher resolution.

inline constexpr int midi1Max = 127;
inline constexpr int midi2Max = 16383;

enum class VelocityOutputMode
{
    autoDetect,
    midi1,
    midi2
};

inline float midi1ToNormalized (int velocity) noexcept
{
    return static_cast<float> (std::clamp (velocity, 0, midi1Max)) / static_cast<float> (midi1Max);
}

inline float midi2ToNormalized (int velocity) noexcept
{
    return static_cast<float> (std::clamp (velocity, 0, midi2Max)) / static_cast<float> (midi2Max);
}

inline int normalizedToMidi1 (float normalized) noexcept
{
    const auto clamped = std::clamp (normalized, 0.0f, 1.0f);
    return static_cast<int> (std::lround (clamped * static_cast<float> (midi1Max)));
}

inline int normalizedToMidi2 (float normalized) noexcept
{
    const auto clamped = std::clamp (normalized, 0.0f, 1.0f);
    return static_cast<int> (std::lround (clamped * static_cast<float> (midi2Max)));
}

inline int downgradeToMidi1 (int midi2Velocity) noexcept
{
    return normalizedToMidi1 (midi2ToNormalized (midi2Velocity));
}

} // namespace svc
