#pragma once

#include "HitEventFifo.h"
#include "MidiVelocity.h"
#include "VelocityCurve.h"
#include <JuceHeader.h>
#include <array>
#include <shared_mutex>
#include <unordered_map>

namespace svc
{

struct PadSettings
{
    int midiNote = 36;
    int midiChannel = 10;
    juce::String name;
    VelocityCurve curve;
    bool enabled = true;
    float velocityGate = 0.0f;
    double retriggerGuardMs = 0.0;
};

class VelocityEngine
{
public:
    VelocityEngine();

    void setSampleRate (double rate) noexcept;
    void setOutputMode (VelocityOutputMode mode) noexcept;
    VelocityOutputMode getOutputMode() const noexcept { return outputMode; }

    void clearAllPads();
    void setPadSettings (int note, int channel, const PadSettings& settings);
    PadSettings getPadSettings (int note, int channel) const;

    void processMidiBuffer (juce::MidiBuffer& buffer, int numSamples);
    HitEventFifo& getHitFifo() noexcept { return hitFifo; }

private:
    struct NoteKey
    {
        int note;
        int channel;

        bool operator== (const NoteKey& other) const noexcept
        {
            return note == other.note && channel == other.channel;
        }
    };

    struct NoteKeyHash
    {
        std::size_t operator() (const NoteKey& key) const noexcept
        {
            return static_cast<std::size_t> (key.note) * 17u + static_cast<std::size_t> (key.channel);
        }
    };

    struct RetriggerState
    {
        double lastNoteOnTime = -1.0;
    };

    using PadMap = std::unordered_map<NoteKey, PadSettings, NoteKeyHash>;
    using RetriggerMap = std::unordered_map<NoteKey, RetriggerState, NoteKeyHash>;

    PadMap pads;
    RetriggerMap retriggerStates;
    mutable std::shared_mutex padMutex;
    VelocityOutputMode outputMode = VelocityOutputMode::autoDetect;
    HitEventFifo hitFifo;
    double sampleRate = 44100.0;
    double runningTimeSeconds = 0.0;

    const PadSettings* findPad (int note, int channel) const;
    float processNoteVelocity (const PadSettings& pad, float inputNormalized) const;
    void applyOutputVelocity (juce::MidiMessage& message, float outputNormalized, bool inputIsMidi2) const;
    bool shouldDropRetrigger (const PadSettings& pad, int note, int channel, double eventTimeSeconds) noexcept;
};

} // namespace svc
