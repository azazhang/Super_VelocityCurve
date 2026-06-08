#pragma once

#include <JuceHeader.h>

namespace svc::ui
{

struct Theme
{
    static constexpr juce::uint32 background       = 0xff0e1117;
    static constexpr juce::uint32 panel            = 0xff171c26;
    static constexpr juce::uint32 panelRaised      = 0xff1f2633;
    static constexpr juce::uint32 border           = 0xff2d3648;
    static constexpr juce::uint32 accent           = 0xff4da3ff;
    static constexpr juce::uint32 accentDim          = 0xff2a5f99;
    static constexpr juce::uint32 accentWarm         = 0xffffb347;
    static constexpr juce::uint32 textPrimary        = 0xfff2f5fa;
    static constexpr juce::uint32 textSecondary      = 0xff9aa8bc;
    static constexpr juce::uint32 padIdle            = 0xff283040;
    static constexpr juce::uint32 padHover           = 0xff334055;
    static constexpr juce::uint32 padSelected       = 0xff3d6faa;
    static constexpr juce::uint32 padHit             = 0xffff8c42;
    static constexpr juce::uint32 padDisabled        = 0xff1a1f28;
    static constexpr juce::uint32 curveLine           = 0xff4da3ff;
    static constexpr juce::uint32 curveGrid           = 0xff2a3344;
    static constexpr juce::uint32 curveHit            = 0xffff8c42;
    static constexpr juce::uint32 success             = 0xff5fd38d;

    static juce::Font titleFont()       { return juce::Font (juce::FontOptions (20.0f)).boldened(); }
    static juce::Font sectionFont()     { return juce::Font (juce::FontOptions (13.0f)).boldened(); }
    static juce::Font bodyFont()        { return juce::Font (juce::FontOptions (12.0f)); }
    static juce::Font smallFont()       { return juce::Font (juce::FontOptions (10.5f)); }

    static void fillPanel (juce::Graphics& g, juce::Rectangle<float> bounds, float radius = 8.0f)
    {
        g.setColour (juce::Colour (panel));
        g.fillRoundedRectangle (bounds, radius);
        g.setColour (juce::Colour (border));
        g.drawRoundedRectangle (bounds, radius, 1.0f);
    }
};

} // namespace svc::ui
