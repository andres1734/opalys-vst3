#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include "PluginProcessor.h"
#include "OpalysLookAndFeel.h"

class OpalysAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                         private juce::Timer
{
public:
    explicit OpalysAudioProcessorEditor(OpalysAudioProcessor&);
    ~OpalysAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawPlasmaVisualizer(juce::Graphics& g, juce::Rectangle<float> area);

    OpalysAudioProcessor& audioProcessor;
    opalys::gui::OpalysLookAndFeel lookAndFeel;

    float plasmaPhase { 0.0f };
    int selectedModuleIndex { 0 };

    juce::Label brandLabel;
    juce::Label pluginTitleLabel;
    juce::TextButton powerButton { "Encendido" };
    juce::ComboBox presetSelector;
    juce::TextButton prevPresetBtn { "<" };
    juce::TextButton nextPresetBtn { ">" };
    juce::TextButton settingsBtn { "Ajustes" };

    juce::Slider macroCuerpo, macroAire, macroSuciedad;
    juce::Slider macroEspacio, macroEco, macroMovimiento;

    struct ModuleTabButton : public juce::Button
    {
        ModuleTabButton(const juce::String& name, int index)
            : juce::Button(name), slotIndex(index) {}
        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
        int slotIndex;
        bool isCurrentActive { false };
        bool isEffectEnabled { true };
    };

    std::array<std::unique_ptr<ModuleTabButton>, 10> rackButtons;

    juce::Slider masterLowCut, masterHighCut, masterVolume;
    juce::TextButton bypassButton { "Bypass" };
    juce::TextButton panicButton  { "Panico" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpalysAudioProcessorEditor)
};