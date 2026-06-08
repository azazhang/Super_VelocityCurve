#pragma once

#include "../Engine/VelocityCurve.h"
#include <JuceHeader.h>
#include <vector>

namespace svc
{

enum class ProfileLayout
{
    gmStandard,
    launchpadDrumRack,
    maschineGroup,
    spdSx,
    fgdp,
    custom
};

struct ProfilePad
{
    int midiNote = 36;
    int midiChannel = 10;
    juce::String label;
    int gridRow = 0;
    int gridCol = 0;
    VelocityCurve curve;
    bool enabled = true;
    float velocityGate = 0.0f;
    double retriggerGuardMs = 0.0;
};

class ControllerProfile
{
public:
    ControllerProfile() = default;
    explicit ControllerProfile (juce::String profileName, ProfileLayout layoutType);

    const juce::String& getName() const noexcept { return name; }
    void setName (const juce::String& newName) { name = newName; }
    ProfileLayout getLayout() const noexcept { return layout; }
    void setLayout (ProfileLayout newLayout) { layout = newLayout; }

    const std::vector<ProfilePad>& getPads() const noexcept { return pads; }
    std::vector<ProfilePad>& getPads() noexcept { return pads; }

    void getGridDimensions (int& rows, int& cols) const noexcept;
    void applyToEngine (class VelocityEngine& engine) const;
    ControllerProfile copy() const;

    juce::ValueTree toValueTree() const;
    static ControllerProfile fromValueTree (const juce::ValueTree& tree);

    static ControllerProfile createGMStandard();
    static ControllerProfile createLaunchpadDrumRack();
    static ControllerProfile createMaschineGroup();
    static ControllerProfile createSpdSx();
    static ControllerProfile createFgdp();

private:
    juce::String name { "GM Standard" };
    ProfileLayout layout = ProfileLayout::gmStandard;
    std::vector<ProfilePad> pads;
};

} // namespace svc
