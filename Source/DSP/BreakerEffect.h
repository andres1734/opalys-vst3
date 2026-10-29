#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace opalys::dsp
{
    class BreakerEffect
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec) { juce::ignoreUnused(spec); }
        void reset() {}
        void processBlock(juce::AudioBuffer<float>& buffer, double ppq) noexcept { juce::ignoreUnused(buffer, ppq); }
    };
}
