#pragma once

#include <JuceHeader.h>
#include <optional>
#include <vector>

namespace svc
{

struct DrumPadDefinition
{
    int midiNote;
    juce::String name;
    int row;
    int col;
};

inline const std::vector<DrumPadDefinition>& getGMDrumPads()
{
    static const std::vector<DrumPadDefinition> pads = {
        { 36, "Kick", 3, 0 },
        { 38, "Snare", 2, 1 },
        { 37, "Side Stick", 2, 0 },
        { 42, "Closed HH", 1, 0 },
        { 46, "Open HH", 0, 0 },
        { 44, "Pedal HH", 1, 1 },
        { 49, "Crash", 0, 2 },
        { 51, "Ride", 0, 3 },
        { 41, "Low Floor Tom", 3, 1 },
        { 43, "High Floor Tom", 3, 2 },
        { 45, "Low Tom", 2, 2 },
        { 47, "Low-Mid Tom", 2, 3 },
        { 48, "Hi-Mid Tom", 1, 2 },
        { 50, "High Tom", 1, 3 },
        { 39, "Hand Clap", 2, 4 },
        { 40, "E-Snare", 3, 4 },
        { 35, "Acoustic BD", 3, 5 },
        { 52, "China", 0, 4 },
        { 53, "Ride Bell", 0, 5 },
        { 54, "Tambourine", 1, 4 },
        { 55, "Splash", 0, 6 },
        { 56, "Cowbell", 1, 5 },
        { 57, "Crash 2", 0, 7 },
        { 58, "Vibraslap", 1, 6 },
        { 59, "Ride 2", 0, 8 }
    };
    return pads;
}

inline juce::String getGMDrumName (int midiNote)
{
    for (const auto& pad : getGMDrumPads())
        if (pad.midiNote == midiNote)
            return pad.name;

    if (midiNote >= 35 && midiNote <= 81)
        return "GM Perc " + juce::String (midiNote);

    return "Note " + juce::String (midiNote);
}

} // namespace svc
