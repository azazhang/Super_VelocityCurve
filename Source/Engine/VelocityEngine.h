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
};

class VelocityEngine
{
public:
    static constexpr int maxNotes = 128;
    static constexpr int maxChannels = 16;

    VelocityEngine();

    void setOutputMode (VelocityOutputMode mode) noexcept;
    VelocityOutputMode getOutputMode() const noexcept { return outputMode; }

    void setPadSettings (int note, int channel, const PadSettings& settings);
    PadSettings getPadSettings (int note, int channel) const;

    void processMidiBuffer (juce::MidiBuffer& buffer, int numSamples);
    HitEventFifo& getHitFifo() noexcept { return hitFifo; }

    void setRetriggerGuardMs (double ms) noexcept { retriggerGuardMs = ms; }
    double getRetriggerGuardMs() const noexcept { return retriggerGuardMs; }

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

    using PadMap = std::unordered_map<NoteKey, PadSettings, NoteKeyHash>;

    PadMap pads;
    mutable std::shared_mutex padMutex;
    VelocityOutputMode outputMode = VelocityOutputMode::autoDetect;
    HitEventFifo hitFifo;
    double retriggerGuardMs = 0.0;
    double sampleRate = 44100.0;
    std::array<double, maxNotes> lastNoteOnTime {};
    std::array<int, maxNotes> lastNoteOnChannel {};

    svc::PadSettings* findPad (int note, int channel);
    const svc::PadSettings* findPad (int note, int channel) const;
    float processNoteVelocity (const PadSettings& pad, float inputNormalized, bool inputIsMidi2) const;
    void applyOutputVelocity (juce::MidiMessage& message, float outputNormalized, bool inputIsMidi2) const;
    bool shouldDropRetrigger (int note, int channel, double eventTimeSeconds) noexcept;
};

} // namespace svc
