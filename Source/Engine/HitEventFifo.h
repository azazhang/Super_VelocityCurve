#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace svc
{

struct HitEvent
{
    int note = 0;
    int channel = 1;
    float inputVelocity = 0.0f;
    float outputVelocity = 0.0f;
    bool isMidi2 = false;
    std::uint64_t timestamp = 0;
};

// Lock-free single-producer / single-consumer ring for audio thread -> UI thread.
class HitEventFifo
{
public:
    static constexpr int capacity = 256;

    bool push (const HitEvent& event) noexcept
    {
        const auto write = writeIndex.load (std::memory_order_relaxed);
        const auto nextWrite = (write + 1) % capacity;

        if (nextWrite == readIndex.load (std::memory_order_acquire))
            return false;

        buffer[static_cast<size_t> (write)] = event;
        writeIndex.store (nextWrite, std::memory_order_release);
        return true;
    }

    bool pop (HitEvent& event) noexcept
    {
        const auto read = readIndex.load (std::memory_order_relaxed);

        if (read == writeIndex.load (std::memory_order_acquire))
            return false;

        event = buffer[static_cast<size_t> (read)];
        readIndex.store ((read + 1) % capacity, std::memory_order_release);
        return true;
    }

    void clear() noexcept
    {
        readIndex.store (0, std::memory_order_release);
        writeIndex.store (0, std::memory_order_release);
    }

private:
    std::array<HitEvent, capacity> buffer {};
    std::atomic<int> writeIndex { 0 };
    std::atomic<int> readIndex { 0 };
};

} // namespace svc
