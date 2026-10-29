#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

#include "DSP/SaturationEffect.h"
#include "DSP/TapeEffect.h"
#include "DSP/GranularSparkles.h"
#include "DSP/DelayEffect.h"
#include "DSP/ReverbEffect.h"
#include "DSP/WiderEffect.h"
#include "DSP/EnhancerEffect.h"
#include "DSP/PitchEffect.h"
#include "DSP/BreakerEffect.h"
#include "DSP/RicochetEffect.h"

class OpalysAudioProcessor final : public juce::AudioProcessor
{
public:
    OpalysAudioProcessor();
    ~OpalysAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) noexcept override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Opalys"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 4.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void triggerPanic() noexcept { panicRequested.store(true, std::memory_order_release); }
    void setModuleOrder(const std::array<uint8_t, 10>& newOrder) noexcept;
    std::array<uint8_t, 10> getModuleOrder() const noexcept;

    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    std::array<std::atomic<uint8_t>, 10> moduleOrder;
    std::atomic<bool> panicRequested { false };

    opalys::dsp::SaturationEffect saturationFX;
    opalys::dsp::TapeEffect       tapeFX;
    opalys::dsp::GranularSparkles sparklesFX;
    opalys::dsp::DelayEffect      delayFX;
    opalys::dsp::ReverbEffect     reverbFX;
    opalys::dsp::WiderEffect      widerFX;
    opalys::dsp::EnhancerEffect   enhancerFX;
    opalys::dsp::PitchEffect      pitchFX;
    opalys::dsp::BreakerEffect    breakerFX;
    opalys::dsp::RicochetEffect   ricochetFX;

    juce::dsp::IIR::Filter<float> masterLowCutL, masterLowCutR;
    juce::dsp::IIR::Filter<float> masterHighCutL, masterHighCutR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpalysAudioProcessor)
};