#include "ProfileStore.h"

namespace svc
{

ProfileStore::ProfileStore()
{
    rebuildFactoryProfiles();
    activeProfile = factoryProfiles.front();
}

void ProfileStore::rebuildFactoryProfiles()
{
    factoryProfiles = {
        ControllerProfile::createGMStandard(),
        ControllerProfile::createLaunchpadDrumRack(),
        ControllerProfile::createMaschineGroup(),
        ControllerProfile::createSpdSx(),
        ControllerProfile::createFgdp()
    };
}

void ProfileStore::setActiveProfileIndex (int index)
{
    if (index >= 0 && index < static_cast<int> (factoryProfiles.size()))
    {
        activeProfileIndex = index;
        activeProfile = factoryProfiles[static_cast<size_t> (index)];
    }
}

juce::ValueTree ProfileStore::toValueTree() const
{
    juce::ValueTree tree ("SuperVelocityCurveProfile");
    tree.setProperty ("version", 1, nullptr);
    tree.setProperty ("activeProfileIndex", activeProfileIndex, nullptr);
    tree.appendChild (activeProfile.toValueTree(), nullptr);
    return tree;
}

void ProfileStore::fromValueTree (const juce::ValueTree& tree)
{
    if (! tree.hasType ("SuperVelocityCurveProfile"))
        return;

    activeProfileIndex = tree.getProperty ("activeProfileIndex", 0);

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto child = tree.getChild (i);
        if (child.hasType ("ControllerProfile"))
            activeProfile = ControllerProfile::fromValueTree (child);
    }
}

bool ProfileStore::exportProfileToFile (const juce::File& file) const
{
    const auto xml = toValueTree().createXml();
    if (xml == nullptr)
        return false;

    return xml->writeTo (file);
}

bool ProfileStore::importProfileFromFile (const juce::File& file)
{
    const auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr)
        return false;

    fromValueTree (juce::ValueTree::fromXml (*xml));
    return true;
}

} // namespace svc
