#include "VelocityEngine.h"
#include <cmath>

namespace svc
{

VelocityEngine::VelocityEngine()
{
    lastNoteOnChannel.fill (0);
    lastNoteOnTime.fill (-1.0);
}

void VelocityEngine::setOutputMode (VelocityOutputMode mode) noexcept
{
    outputMode = mode;
}

void VelocityEngine::setPadSettings (int note, int channel, const PadSettings& settings)
{
    const std::unique_lock lock (padMutex);
    pads[{ note, channel }] = settings;
}

PadSettings VelocityEngine::getPadSettings (int note, int channel) const
{
    const std::shared_lock lock (padMutex);
    const NoteKey key { note, channel };
    const auto it = pads.find (key);
    if (it != pads.end())
        return it->second;

    PadSettings defaults;
    defaults.midiNote = note;
    defaults.midiChannel = channel;
    defaults.name = "Note " + juce::String (note);
    return defaults;
}

PadSettings* VelocityEngine::findPad (int note, int channel)
{
    const NoteKey key { note, channel };
    const auto it = pads.find (key);
    return it != pads.end() ? &it->second : nullptr;
}

const PadSettings* VelocityEngine::findPad (int note, int channel) const
{
    const NoteKey key { note, channel };
    const auto it = pads.find (key);
    return it != pads.end() ? &it->second : nullptr;
}

bool VelocityEngine::shouldDropRetrigger (int note, int channel, double eventTimeSeconds) noexcept
{
    if (retriggerGuardMs <= 0.0)
        return false;

    if (lastNoteOnChannel[static_cast<size_t> (note)] != channel)
        return false;

    const auto lastTime = lastNoteOnTime[static_cast<size_t> (note)];
    if (lastTime < 0.0)
        return false;

    const auto guardSeconds = retriggerGuardMs * 0.001;
    return (eventTimeSeconds - lastTime) < guardSeconds;
}

float VelocityEngine::processNoteVelocity (const PadSettings& pad,
                                           float inputNormalized,
                                           bool inputIsMidi2) const
{
    if (! pad.enabled)
        return inputNormalized;

    if (inputNormalized < pad.velocityGate)
        return -1.0f;

    return pad.curve.mapNormalized (inputNormalized);
}

void VelocityEngine::applyOutputVelocity (juce::MidiMessage& message,
                                          float outputNormalized,
                                          bool inputIsMidi2) const
{
    const auto mode = outputMode;
    const auto useMidi2 = mode == VelocityOutputMode::midi2
                          || (mode == VelocityOutputMode::autoDetect && inputIsMidi2);

    if (useMidi2)
    {
        // JUCE currently exposes velocity as normalized float; hosts upgrade to MIDI 2.0 as needed.
        message.setVelocity (outputNormalized);
    }
    else
    {
        message.setVelocity (outputNormalized);
    }
}

void VelocityEngine::processMidiBuffer (juce::MidiBuffer& buffer, int numSamples)
{
    juce::MidiBuffer processed;
    const auto blockDurationSeconds = static_cast<double> (numSamples) / sampleRate;
    static double runningTime = 0.0;

    std::shared_lock lock (padMutex);

    for (const auto metadata : buffer)
    {
        auto message = metadata.getMessage();
        const auto sampleOffset = metadata.samplePosition;
        const auto eventTime = runningTime + (static_cast<double> (sampleOffset) / sampleRate);

        if (message.isNoteOn())
        {
            const auto note = message.getNoteNumber();
            const auto channel = message.getChannel();
            const auto inputNormalized = message.getFloatVelocity();

            // MIDI 2.0 detection: when JUCE exposes higher-resolution attributes on the message,
            // treat non-quantized velocities as MIDI 2.0 for output-mode Auto.
            const auto midi1Quantized = midi1ToNormalized (message.getVelocity());
            const bool inputIsMidi2 = std::abs (inputNormalized - midi1Quantized) > (1.0f / 254.0f);

            if (shouldDropRetrigger (note, channel, eventTime))
                continue;

            const auto* pad = findPad (note, channel);
            PadSettings settings;
            if (pad != nullptr)
            {
                settings = *pad;
            }
            else
            {
                settings.midiNote = note;
                settings.midiChannel = channel;
                settings.name = "Note " + juce::String (note);
            }

            const auto outputNormalized = processNoteVelocity (settings, inputNormalized, inputIsMidi2);
            if (outputNormalized < 0.0f)
                continue;

            applyOutputVelocity (message, outputNormalized, inputIsMidi2);

            HitEvent hit;
            hit.note = note;
            hit.channel = channel;
            hit.inputVelocity = inputNormalized;
            hit.outputVelocity = outputNormalized;
            hit.isMidi2 = inputIsMidi2;
            hit.timestamp = static_cast<std::uint64_t> (eventTime * 1000.0);
            hitFifo.push (hit);

            lastNoteOnTime[static_cast<size_t> (note)] = eventTime;
            lastNoteOnChannel[static_cast<size_t> (note)] = channel;
        }

        processed.addEvent (message, sampleOffset);
    }

    lock.unlock();
    buffer.swapWith (processed);
    runningTime += blockDurationSeconds;
}

} // namespace svc
