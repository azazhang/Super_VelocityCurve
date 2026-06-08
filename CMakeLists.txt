cmake_minimum_required(VERSION 3.22)

project(SuperVelocityCurve VERSION 0.2.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

# IS_MIDI_EFFECT FALSE + stereo output + Instrument category: required for Ableton Live VST3 visibility.
# AU builds as a Music Effect (aumf) — use VST3 in Ableton for MIDI routing.
juce_add_plugin(SuperVelocityCurve
    COMPANY_NAME "SuperVelocityCurve"
    PLUGIN_MANUFACTURER_CODE Svvc
    PLUGIN_CODE Svcv
    BUNDLE_ID com.supervelocitycurve.plugin
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Super VelocityCurve"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    VST3_CATEGORIES Instrument
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
        Source/UI/CurveEditorComponent.cpp
        Source/UI/PadInspectorComponent.cpp)

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

option(SVC_BUILD_TESTS "Build engine unit tests" ON)

if(SVC_BUILD_TESTS)
    enable_testing()

    juce_add_console_app(SuperVelocityCurveTests
        PRODUCT_NAME "SuperVelocityCurveTests")

    juce_generate_juce_header(SuperVelocityCurveTests)

    target_sources(SuperVelocityCurveTests
        PRIVATE
            Tests/EngineTests.cpp
            Source/Engine/VelocityCurve.cpp
            Source/Engine/VelocityEngine.cpp
            Source/Profiles/ControllerProfile.cpp
            Source/Profiles/ProfileStore.cpp)

    target_include_directories(SuperVelocityCurveTests PRIVATE Source)

    target_link_libraries(SuperVelocityCurveTests PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_data_structures
        juce::juce_recommended_warning_flags)

    add_test(NAME EngineTests COMMAND SuperVelocityCurveTests)
endif()
