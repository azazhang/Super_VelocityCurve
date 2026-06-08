#include "ControllerProfile.h"
#include "../Engine/VelocityEngine.h"
#include "GMDrumMap.h"

namespace svc
{

namespace
{
juce::ValueTree curveToTree (const VelocityCurve& curve)
{
    juce::ValueTree tree ("Curve");
    tree.setProperty ("floor", curve.getFloor(), nullptr);
    tree.setProperty ("ceiling", curve.getCeiling(), nullptr);

    for (const auto& point : curve.getControlPoints())
    {
        juce::ValueTree pointTree ("Point");
        pointTree.setProperty ("input", point.input, nullptr);
        pointTree.setProperty ("output", point.output, nullptr);
        tree.appendChild (pointTree, nullptr);
    }

    return tree;
}

VelocityCurve curveFromTree (const juce::ValueTree& tree)
{
    VelocityCurve curve;
    curve.setFloor (tree.getProperty ("floor", 0.0f));
    curve.setCeiling (tree.getProperty ("ceiling", 1.0f));

    std::vector<CurveControlPoint> points;
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto pointTree = tree.getChild (i);
        if (pointTree.hasType ("Point"))
        {
            points.push_back ({
                static_cast<float> (pointTree.getProperty ("input")),
                static_cast<float> (pointTree.getProperty ("output"))
            });
        }
    }

    if (points.size() >= 2)
        curve.setControlPoints (points);

    return curve;
}
} // namespace

ControllerProfile::ControllerProfile (juce::String profileName, ProfileLayout layoutType)
    : name (std::move (profileName)), layout (layoutType)
{
}

void ControllerProfile::getGridDimensions (int& rows, int& cols) const noexcept
{
    rows = 1;
    cols = 1;

    for (const auto& pad : pads)
    {
        rows = std::max (rows, pad.gridRow + 1);
        cols = std::max (cols, pad.gridCol + 1);
    }
}

ControllerProfile ControllerProfile::copy() const
{
    ControllerProfile copy;
    copy.name = name;
    copy.layout = layout;
    copy.pads = pads;
    return copy;
}

void ControllerProfile::applyToEngine (VelocityEngine& engine) const
{
    engine.clearAllPads();

    for (const auto& pad : pads)
    {
        PadSettings settings;
        settings.midiNote = pad.midiNote;
        settings.midiChannel = pad.midiChannel;
        settings.name = pad.label;
        settings.curve = pad.curve;
        settings.enabled = pad.enabled;
        settings.velocityGate = pad.velocityGate;
        settings.retriggerGuardMs = pad.retriggerGuardMs;
        engine.setPadSettings (pad.midiNote, pad.midiChannel, settings);
    }
}

juce::ValueTree ControllerProfile::toValueTree() const
{
    juce::ValueTree tree ("ControllerProfile");
    tree.setProperty ("name", name, nullptr);
    tree.setProperty ("layout", static_cast<int> (layout), nullptr);

    for (const auto& pad : pads)
    {
        juce::ValueTree padTree ("Pad");
        padTree.setProperty ("midiNote", pad.midiNote, nullptr);
        padTree.setProperty ("midiChannel", pad.midiChannel, nullptr);
        padTree.setProperty ("label", pad.label, nullptr);
        padTree.setProperty ("gridRow", pad.gridRow, nullptr);
        padTree.setProperty ("gridCol", pad.gridCol, nullptr);
        padTree.setProperty ("enabled", pad.enabled, nullptr);
        padTree.setProperty ("velocityGate", pad.velocityGate, nullptr);
        padTree.setProperty ("retriggerGuardMs", pad.retriggerGuardMs, nullptr);
        padTree.appendChild (curveToTree (pad.curve), nullptr);
        tree.appendChild (padTree, nullptr);
    }

    return tree;
}

ControllerProfile ControllerProfile::fromValueTree (const juce::ValueTree& tree)
{
    ControllerProfile profile;
    profile.name = tree.getProperty ("name", "Custom");
    profile.layout = static_cast<ProfileLayout> (static_cast<int> (tree.getProperty ("layout", 0)));

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const auto padTree = tree.getChild (i);
        if (! padTree.hasType ("Pad"))
            continue;

        ProfilePad pad;
        pad.midiNote = padTree.getProperty ("midiNote", 36);
        pad.midiChannel = padTree.getProperty ("midiChannel", 10);
        pad.label = padTree.getProperty ("label", juce::String());
        pad.gridRow = padTree.getProperty ("gridRow", 0);
        pad.gridCol = padTree.getProperty ("gridCol", 0);
        pad.enabled = padTree.getProperty ("enabled", true);
        pad.velocityGate = padTree.getProperty ("velocityGate", 0.0f);
        pad.retriggerGuardMs = padTree.getProperty ("retriggerGuardMs", 0.0);

        for (int c = 0; c < padTree.getNumChildren(); ++c)
        {
            const auto child = padTree.getChild (c);
            if (child.hasType ("Curve"))
                pad.curve = curveFromTree (child);
        }

        profile.pads.push_back (pad);
    }

    return profile;
}

ControllerProfile ControllerProfile::createGMStandard()
{
    ControllerProfile profile ("GM Standard", ProfileLayout::gmStandard);

    for (const auto& def : getGMDrumPads())
    {
        ProfilePad pad;
        pad.midiNote = def.midiNote;
        pad.midiChannel = 10;
        pad.label = def.name;
        pad.gridRow = def.row;
        pad.gridCol = def.col;
        profile.pads.push_back (pad);
    }

    return profile;
}

ControllerProfile ControllerProfile::createLaunchpadDrumRack()
{
    ControllerProfile profile ("Launchpad Drum Rack", ProfileLayout::launchpadDrumRack);

    int note = 36;
    for (int row = 3; row >= 0; --row)
    {
        for (int col = 0; col < 4; ++col)
        {
            ProfilePad pad;
            pad.midiNote = note;
            pad.midiChannel = 1;
            pad.label = getGMDrumName (note);
            pad.gridRow = row;
            pad.gridCol = col;
            profile.pads.push_back (pad);
            ++note;
        }
    }

    return profile;
}

ControllerProfile ControllerProfile::createMaschineGroup()
{
    ControllerProfile profile ("Maschine Group", ProfileLayout::maschineGroup);

    int note = 36;
    for (int row = 3; row >= 0; --row)
    {
        for (int col = 0; col < 4; ++col)
        {
            ProfilePad pad;
            pad.midiNote = note;
            pad.midiChannel = 1;
            pad.label = "Slot " + juce::String (note - 35);
            pad.gridRow = row;
            pad.gridCol = col;
            profile.pads.push_back (pad);
            ++note;
        }
    }

    return profile;
}

ControllerProfile ControllerProfile::createSpdSx()
{
    ControllerProfile profile ("Roland SPD-SX", ProfileLayout::spdSx);

    int note = 60;
    for (int i = 0; i < 12; ++i)
    {
        ProfilePad pad;
        pad.midiNote = note + i;
        pad.midiChannel = 10;
        pad.label = "Pad " + juce::String (i + 1);
        pad.gridRow = i / 4;
        pad.gridCol = i % 4;
        profile.pads.push_back (pad);
    }

    return profile;
}

ControllerProfile ControllerProfile::createFgdp()
{
    ControllerProfile profile ("Yamaha FGDP", ProfileLayout::fgdp);

    const int notes[] = { 36, 38, 42, 46, 41, 43, 45, 47, 49, 51, 37, 40 };
    const char* labels[] = {
        "Kick", "Snare", "Closed HH", "Open HH",
        "Low Tom", "Mid Tom", "High Tom", "Floor Tom",
        "Crash", "Ride", "Rim", "Clap"
    };

    for (int i = 0; i < 12; ++i)
    {
        ProfilePad pad;
        pad.midiNote = notes[i];
        pad.midiChannel = 3;
        pad.label = labels[i];
        pad.gridRow = i / 4;
        pad.gridCol = i % 4;
        profile.pads.push_back (pad);
    }

    return profile;
}

} // namespace svc
