#pragma once

#include "../Profiles/ControllerProfile.h"
#include "Theme.h"
#include <JuceHeader.h>
#include <functional>

class PadInspectorComponent : public juce::Component
{
public:
    PadInspectorComponent();

    void setPad (const svc::ProfilePad& pad, int padIndex);
    int getPadIndex() const noexcept { return currentPadIndex; }

    std::function<void (int padIndex, const svc::ProfilePad&)> onPadChanged;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    svc::ProfilePad currentPad;
    int currentPadIndex = 0;

    juce::ToggleButton enabledToggle { "Pad enabled" };
    juce::Slider velocityGateSlider;
    juce::Slider retriggerSlider;
    juce::Slider floorSlider;
    juce::Slider ceilingSlider;
    juce::Label gateLabel { {}, "Velocity gate" };
    juce::Label retriggerLabel { {}, "Retrigger guard (ms)" };
    juce::Label floorLabel { {}, "Output floor" };
    juce::Label ceilingLabel { {}, "Output ceiling" };

    void notifyChanged();
    void setupSlider (juce::Slider& slider, const juce::String& suffix);
};
