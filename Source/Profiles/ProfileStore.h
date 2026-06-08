#pragma once

#include "ControllerProfile.h"
#include <JuceHeader.h>
#include <functional>
#include <vector>

namespace svc
{

enum class ProfileEntryType
{
    factoryTemplate,
    userProfile
};

struct ProfileListEntry
{
    ProfileEntryType type = ProfileEntryType::factoryTemplate;
    int index = 0;
    juce::String displayName;
};

class ProfileStore
{
public:
    ProfileStore();

    const std::vector<ControllerProfile>& getFactoryTemplates() const noexcept { return factoryTemplates; }
    const std::vector<ControllerProfile>& getUserProfiles() const noexcept { return userProfiles; }

    ControllerProfile& getActiveProfile() noexcept { return activeProfile; }
    const ControllerProfile& getActiveProfile() const noexcept { return activeProfile; }

    ProfileEntryType getActiveEntryType() const noexcept { return activeEntryType; }
    int getActiveEntryIndex() const noexcept { return activeEntryIndex; }

    std::vector<ProfileListEntry> getProfileList() const;
    void loadFactoryTemplate (int index);
    void loadUserProfile (int index);
    bool saveActiveAsUserProfile (const juce::String& name);
    bool deleteUserProfile (int index);
    bool duplicateActiveAsUserProfile (const juce::String& name);
    void applyActiveToEngine (class VelocityEngine& engine) const;

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree& tree);

    bool exportActiveProfileToFile (const juce::File& file) const;
    bool importProfileFromFile (const juce::File& file);
    void syncActiveUserProfileFromEdits();

    std::function<void()> onProfileChanged;

private:
    std::vector<ControllerProfile> factoryTemplates;
    std::vector<ControllerProfile> userProfiles;
    ControllerProfile activeProfile;
    ProfileEntryType activeEntryType = ProfileEntryType::factoryTemplate;
    int activeEntryIndex = 0;

    void rebuildFactoryTemplates();
    void notifyChanged();
    void setActiveFromEntry (ProfileEntryType type, int index);
};

} // namespace svc
