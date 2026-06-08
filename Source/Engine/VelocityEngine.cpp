#include "VelocityEngine.h"

namespace svc
{

VelocityEngine::VelocityEngine() = default;

void VelocityEngine::setSampleRate (double rate) noexcept
{
    sampleRate = rate > 0.0 ? rate : 44100.0;
}

void VelocityEngine::setOutputMode (VelocityOutputMode mode) noexcept
{
    outputMode = mode;
}

void VelocityEngine::clearAllPads()
{
    const std::unique_lock lock (padMutex);
    pads.clear();
    retriggerStates.clear();
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

const PadSettings* VelocityEngine::findPad (int note, int channel) const
{
    const NoteKey key { note, channel };
    const auto it = pads.find (key);
    return it != pads.end() ? &it->second : nullptr;
}

bool VelocityEngine::shouldDropRetrigger (const PadSettings& pad,
                                          int note,
                                          int channel,
                                          double eventTimeSeconds) noexcept
{
    if (pad.retriggerGuardMs <= 0.0)
        return false;

    const NoteKey key { note, channel };
    const auto it = retriggerStates.find (key);
    if (it == retriggerStates.end() || it->second.lastNoteOnTime < 0.0)
        return false;

    const auto guardSeconds = pad.retriggerGuardMs * 0.001;
    return (eventTimeSeconds - it->second.lastNoteOnTime) < guardSeconds;
}

float VelocityEngine::processNoteVelocity (const PadSettings& pad, float inputNormalized) const
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
    const auto forceMidi1 = mode == VelocityOutputMode::midi1;
    const auto forceMidi2 = mode == VelocityOutputMode::midi2;
    const auto useMidi1 = forceMidi1 || (! forceMidi2 && ! inputIsMidi2);

    if (useMidi1)
    {
        const auto midi1 = normalizedToMidi1 (outputNormalized);
        message.setVelocity (midi1ToNormalized (midi1));
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

    std::shared_lock lock (padMutex);

    for (const auto metadata : buffer)
    {
        auto message = metadata.getMessage();
        const auto sampleOffset = metadata.samplePosition;
        const auto eventTime = runningTimeSeconds + (static_cast<double> (sampleOffset) / sampleRate);

        if (message.isNoteOn())
        {
            const auto note = message.getNoteNumber();
            const auto channel = message.getChannel();
            const auto inputNormalized = message.getFloatVelocity();

            const auto midi1Quantized = midi1ToNormalized (message.getVelocity());
            const bool inputIsMidi2 = std::abs (inputNormalized - midi1Quantized) > (1.0f / 254.0f);

            PadSettings settings;
            if (const auto* pad = findPad (note, channel))
                settings = *pad;
            else
            {
                settings.midiNote = note;
                settings.midiChannel = channel;
                settings.name = "Note " + juce::String (note);
            }

            if (shouldDropRetrigger (settings, note, channel, eventTime))
                continue;

            const auto outputNormalized = processNoteVelocity (settings, inputNormalized);
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

            retriggerStates[{ note, channel }].lastNoteOnTime = eventTime;
        }

        processed.addEvent (message, sampleOffset);
    }

    lock.unlock();
    buffer.swapWith (processed);
    runningTimeSeconds += blockDurationSeconds;
}

} // namespace svc
