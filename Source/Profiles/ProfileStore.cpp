#include "ProfileStore.h"
#include "../Engine/VelocityEngine.h"

namespace svc
{

ProfileStore::ProfileStore()
{
    rebuildFactoryTemplates();
    activeProfile = factoryTemplates.front().copy();
}

void ProfileStore::rebuildFactoryTemplates()
{
    factoryTemplates = {
        ControllerProfile::createGMStandard(),
        ControllerProfile::createLaunchpadDrumRack(),
        ControllerProfile::createMaschineGroup(),
        ControllerProfile::createSpdSx(),
        ControllerProfile::createFgdp()
    };
}

void ProfileStore::notifyChanged()
{
    if (onProfileChanged)
        onProfileChanged();
}

std::vector<ProfileListEntry> ProfileStore::getProfileList() const
{
    std::vector<ProfileListEntry> entries;

    for (int i = 0; i < static_cast<int> (factoryTemplates.size()); ++i)
    {
        entries.push_back ({
            ProfileEntryType::factoryTemplate,
            i,
            "[Template] " + factoryTemplates[static_cast<size_t> (i)].getName()
        });
    }

    for (int i = 0; i < static_cast<int> (userProfiles.size()); ++i)
    {
        entries.push_back ({
            ProfileEntryType::userProfile,
            i,
            "[My] " + userProfiles[static_cast<size_t> (i)].getName()
        });
    }

    return entries;
}

void ProfileStore::setActiveFromEntry (ProfileEntryType type, int index)
{
    activeEntryType = type;
    activeEntryIndex = index;

    if (type == ProfileEntryType::factoryTemplate
        && index >= 0
        && index < static_cast<int> (factoryTemplates.size()))
    {
        activeProfile = factoryTemplates[static_cast<size_t> (index)].copy();
    }
    else if (type == ProfileEntryType::userProfile
             && index >= 0
             && index < static_cast<int> (userProfiles.size()))
    {
        activeProfile = userProfiles[static_cast<size_t> (index)].copy();
    }

    notifyChanged();
}

void ProfileStore::loadFactoryTemplate (int index)
{
    setActiveFromEntry (ProfileEntryType::factoryTemplate, index);
}

void ProfileStore::loadUserProfile (int index)
{
    setActiveFromEntry (ProfileEntryType::userProfile, index);
}

bool ProfileStore::saveActiveAsUserProfile (const juce::String& name)
{
    if (name.trim().isEmpty())
        return false;

    auto profile = activeProfile.copy();
    profile.setName (name.trim());
    profile.setLayout (ProfileLayout::custom);
    userProfiles.push_back (profile);
    setActiveFromEntry (ProfileEntryType::userProfile, static_cast<int> (userProfiles.size()) - 1);
    return true;
}

bool ProfileStore::deleteUserProfile (int index)
{
    if (index < 0 || index >= static_cast<int> (userProfiles.size()))
        return false;

    userProfiles.erase (userProfiles.begin() + index);

    if (activeEntryType == ProfileEntryType::userProfile)
    {
        if (userProfiles.empty())
            loadFactoryTemplate (0);
        else
            loadUserProfile (juce::jmin (index, static_cast<int> (userProfiles.size()) - 1));
    }

    notifyChanged();
    return true;
}

bool ProfileStore::duplicateActiveAsUserProfile (const juce::String& name)
{
    return saveActiveAsUserProfile (name.isEmpty() ? activeProfile.getName() + " Copy" : name);
}

void ProfileStore::applyActiveToEngine (VelocityEngine& engine) const
{
    activeProfile.applyToEngine (engine);
}

juce::ValueTree ProfileStore::toValueTree() const
{
    juce::ValueTree tree ("SuperVelocityCurveProfileStore");
    tree.setProperty ("version", 2, nullptr);
    tree.setProperty ("activeEntryType", static_cast<int> (activeEntryType), nullptr);
    tree.setProperty ("activeEntryIndex", activeEntryIndex, nullptr);
    tree.appendChild (activeProfile.toValueTree(), nullptr);

    juce::ValueTree users ("UserProfiles");
    for (const auto& profile : userProfiles)
        users.appendChild (profile.toValueTree(), nullptr);

    tree.appendChild (users, nullptr);
    return tree;
}

void ProfileStore::syncActiveUserProfileFromEdits()
{
    if (activeEntryType == ProfileEntryType::userProfile
        && activeEntryIndex >= 0
        && activeEntryIndex < static_cast<int> (userProfiles.size()))
    {
        userProfiles[static_cast<size_t> (activeEntryIndex)] = activeProfile.copy();
    }
}

void ProfileStore::fromValueTree (const juce::ValueTree& tree)
{
    if (tree.hasType ("SuperVelocityCurveProfile"))
    {
        activeProfile = ControllerProfile::fromValueTree (tree);
        activeEntryType = ProfileEntryType::factoryTemplate;
        activeEntryIndex = static_cast<int> (tree.getProperty ("activeProfileIndex", 0));
        notifyChanged();
        return;
    }

    if (! tree.hasType ("SuperVelocityCurveProfileStore"))
        return;

    userProfiles.clear();

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto child = tree.getChild (i);
        if (child.hasType ("UserProfiles"))
        {
            for (int u = 0; u < child.getNumChildren(); ++u)
            {
                const auto profileTree = child.getChild (u);
                if (profileTree.hasType ("ControllerProfile"))
                    userProfiles.push_back (ControllerProfile::fromValueTree (profileTree));
            }
        }
        else if (child.hasType ("ControllerProfile"))
        {
            activeProfile = ControllerProfile::fromValueTree (child);
        }
    }

    activeEntryType = static_cast<ProfileEntryType> (static_cast<int> (tree.getProperty ("activeEntryType", 0)));
    activeEntryIndex = tree.getProperty ("activeEntryIndex", 0);

    notifyChanged();
}

bool ProfileStore::exportActiveProfileToFile (const juce::File& file) const
{
    const auto xml = activeProfile.toValueTree().createXml();
    if (xml == nullptr)
        return false;

    return xml->writeTo (file);
}

bool ProfileStore::importProfileFromFile (const juce::File& file)
{
    const auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return false;

    auto profile = ControllerProfile::fromValueTree (juce::ValueTree::fromXml (*xml));
    profile.setLayout (ProfileLayout::custom);
    userProfiles.push_back (profile);
    loadUserProfile (static_cast<int> (userProfiles.size()) - 1);
    return true;
}

} // namespace svc
