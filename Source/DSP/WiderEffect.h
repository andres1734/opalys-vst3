#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

namespace opalys::dsp
{
    class EffectBase
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec) { juce::ignoreUnused(spec); }
        void reset() {}
        void processBlock(juce::AudioBuffer<float>& buffer) noexcept { juce::ignoreUnused(buffer); }
    };

    // Definiciones individuales para que coincidan con el procesador
    class SaturationEffect : public EffectBase {};
    class TapeEffect       : public EffectBase {};
    class GranularSparkles : public EffectBase {};
    class DelayEffect      : public EffectBase {};
    class ReverbEffect     : public EffectBase {};
    class WiderEffect      : public EffectBase {};
    class EnhancerEffect   : public EffectBase {};
    class PitchEffect      : public EffectBase {};
    class RicochetEffect   : public EffectBase {};
}
