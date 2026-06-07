#pragma once

#include "../Profiles/ControllerProfile.h"
#include <JuceHeader.h>
#include <functional>

class PadGridComponent : public juce::Component
{
public:
    explicit PadGridComponent (const svc::ControllerProfile& profile);

    void setProfile (const svc::ControllerProfile& profile);
    void setSelectedPadIndex (int index);
    int getSelectedPadIndex() const noexcept { return selectedPadIndex; }

    std::function<void (int padIndex)> onPadSelected;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;

private:
    svc::ControllerProfile currentProfile;
    int selectedPadIndex = 0;

    juce::Rectangle<int> padBoundsForIndex (int index) const;
};
