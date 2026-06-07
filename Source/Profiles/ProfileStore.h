#pragma once

#include "ControllerProfile.h"
#include <JuceHeader.h>
#include <vector>

namespace svc
{

class ProfileStore
{
public:
    ProfileStore();

    const std::vector<ControllerProfile>& getFactoryProfiles() const noexcept { return factoryProfiles; }
    ControllerProfile& getActiveProfile() noexcept { return activeProfile; }
    const ControllerProfile& getActiveProfile() const noexcept { return activeProfile; }

    void setActiveProfileIndex (int index);
    int getActiveProfileIndex() const noexcept { return activeProfileIndex; }

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& tree);

    bool exportProfileToFile (const juce::File& file) const;
    bool importProfileFromFile (const juce::File& file);

private:
    std::vector<ControllerProfile> factoryProfiles;
    ControllerProfile activeProfile;
    int activeProfileIndex = 0;

    void rebuildFactoryProfiles();
};

} // namespace svc
