cmake_minimum_required(VERSION 3.24)
project(Opalys VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Integración con JUCE 8 mediante FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(Opalys
    COMPANY_NAME                "Midilatino DSP Lab"
    PLUGIN_MANUFACTURER_CODE    Mdlt
    PLUGIN_CODE                 Oply
    FORMATS                     VST3 Standalone
    PRODUCT_NAME                "Opalys"
    IS_SYNTH                     FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_CATEGORIES             "Fx" "Multi-Effects" "Mastering"
)

target_sources(Opalys
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/OpalysLookAndFeel.h
        Source/DSP/SaturationEffect.h
        Source/DSP/TapeEffect.h
        Source/DSP/GranularSparkles.h
        Source/DSP/DelayEffect.h
        Source/DSP/ReverbEffect.h
        Source/DSP/WiderEffect.h
        Source/DSP/EnhancerEffect.h
        Source/DSP/PitchEffect.h
        Source/DSP/BreakerEffect.h
        Source/DSP/RicochetEffect.h
)

target_include_directories(Opalys
    PRIVATE
        Source
        Source/DSP
)

target_compile_definitions(Opalys
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

if (MSVC)
    target_compile_options(Opalys PRIVATE /W4 /arch:AVX2 /fp:fast /utf-8)
else()
    target_compile_options(Opalys PRIVATE -Wall -Wextra -O3 -ffast-math)
endif()

target_link_libraries(Opalys
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)