#include "../Source/Engine/VelocityCurve.h"
#include "../Source/Engine/VelocityEngine.h"
#include "../Source/Profiles/ControllerProfile.h"
#include <cmath>
#include <iostream>

#define EXPECT_TRUE(expr) do { if (! (expr)) { std::cerr << "FAIL: " #expr << " at " << __LINE__ << '\n'; return 1; } } while (0)
#define EXPECT_NEAR(a, b, eps) do { if (std::abs ((a) - (b)) > (eps)) { std::cerr << "FAIL: " #a " vs " #b << " at " << __LINE__ << '\n'; return 1; } } while (0)

int testVelocityCurveMonotonic()
{
    svc::VelocityCurve curve;
    curve.applyPreset (svc::CurvePreset::soft);
    const auto& lut = curve.getLut();

    for (int i = 1; i < svc::VelocityCurve::lutSize; ++i)
        EXPECT_TRUE (lut[static_cast<size_t> (i)] >= lut[static_cast<size_t> (i - 1)]);

    return 0;
}

int testVelocityEnginePerPadRetrigger()
{
    svc::VelocityEngine engine;
    engine.setSampleRate (48000.0);

    svc::PadSettings pad;
    pad.retriggerGuardMs = 50.0;
    pad.enabled = true;
    engine.setPadSettings (36, 10, pad);

    juce::MidiBuffer buffer;
    buffer.addEvent (juce::MidiMessage::noteOn (10, 36, 0.8f), 0);
    engine.processMidiBuffer (buffer, 128);

  buffer.clear();
    buffer.addEvent (juce::MidiMessage::noteOn (10, 36, 0.9f), 0);
    engine.processMidiBuffer (buffer, 128);

    EXPECT_TRUE (buffer.getNumEvents() == 0);
    return 0;
}

int testProfileApplyClearsOldPads()
{
    svc::VelocityEngine engine;
    svc::ControllerProfile gm = svc::ControllerProfile::createGMStandard();
    gm.applyToEngine (engine);

    svc::PadSettings gatedPad;
    gatedPad.velocityGate = 0.5f;
    engine.setPadSettings (36, 10, gatedPad);

    juce::MidiBuffer buffer;
    buffer.addEvent (juce::MidiMessage::noteOn (10, 36, 0.2f), 0);
    engine.processMidiBuffer (buffer, 64);
    EXPECT_TRUE (buffer.getNumEvents() == 0);

    svc::ControllerProfile small = svc::ControllerProfile::createSpdSx();
    small.applyToEngine (engine);

    buffer.clear();
    buffer.addEvent (juce::MidiMessage::noteOn (10, 36, 0.2f), 0);
    engine.processMidiBuffer (buffer, 64);
    EXPECT_TRUE (buffer.getNumEvents() == 1);
    return 0;
}

int main()
{
    if (testVelocityCurveMonotonic() != 0) return 1;
    if (testVelocityEnginePerPadRetrigger() != 0) return 1;
    if (testProfileApplyClearsOldPads() != 0) return 1;
    std::cout << "All engine tests passed.\n";
    return 0;
}
