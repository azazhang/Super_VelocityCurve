cmake_minimum_required(VERSION 3.22)

project(SuperVelocityCurve VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(SuperVelocityCurve
    COMPANY_NAME "SuperVelocityCurve"
    PLUGIN_MANUFACTURER_CODE Svvc
    PLUGIN_CODE Svcv
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Super VelocityCurve"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT TRUE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE)

juce_generate_juce_header(SuperVelocityCurve)

target_sources(SuperVelocityCurve
    PRIVATE
        Source/Engine/VelocityCurve.cpp
        Source/Engine/VelocityEngine.cpp
        Source/Profiles/ControllerProfile.cpp
        Source/Profiles/ProfileStore.cpp
        Source/Plugin/PluginProcessor.cpp
        Source/Plugin/PluginEditor.cpp
        Source/UI/PadGridComponent.cpp
        Source/UI/CurveEditorComponent.cpp)

target_compile_definitions(SuperVelocityCurve
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(SuperVelocityCurve
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
