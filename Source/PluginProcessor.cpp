#include "PluginProcessor.h"
#include "PluginEditor.h"

OpalysAudioProcessor::OpalysAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "OPALYS_STATE", createParameterLayout())
{
    for (uint8_t i = 0; i < 10; ++i)
        moduleOrder[i].store(i, std::memory_order_relaxed);
}

juce::AudioProcessorValueTreeState::ParameterLayout OpalysAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_cuerpo",     "Cuerpo",     -100.0f, 100.0f, 22.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_aire",       "Aire",       -100.0f, 100.0f, 68.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_suciedad",   "Suciedad",   -100.0f, 100.0f, 12.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_espacio",    "Espacio",    -100.0f, 100.0f, 84.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_eco",        "Eco",        -100.0f, 100.0f, 52.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("macro_movimiento", "Movimiento", -100.0f, 100.0f, 44.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_lowcut",  "Corte Graves",    20.0f, 400.0f, 65.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_highcut", "Corte Agudos",  2000.0f, 20000.0f, 18200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("master_gain",    "Volumen General", -24.0f, 12.0f, 0.8f));
    params.push_back(std::make_unique<juce::AudioParameterBool> ("master_bypass",  "Master Bypass",  false));

    return { params.begin(), params.end() };
}

void OpalysAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate        = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32>(getTotalNumOutputChannels());

    saturationFX.prepare(spec); tapeFX.prepare(spec); sparklesFX.prepare(spec);
    delayFX.prepare(spec); reverbFX.prepare(spec); widerFX.prepare(spec);
    enhancerFX.prepare(spec); pitchFX.prepare(spec); breakerFX.prepare(spec); ricochetFX.prepare(spec);

    masterLowCutL.prepare(spec);  masterLowCutR.prepare(spec);
    masterHighCutL.prepare(spec); masterHighCutR.prepare(spec);
}

void OpalysAudioProcessor::releaseResources() {}

bool OpalysAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void OpalysAudioProcessor::setModuleOrder(const std::array<uint8_t, 10>& newOrder) noexcept
{
    for (size_t i = 0; i < 10; ++i)
        moduleOrder[i].store(newOrder[i], std::memory_order_release);
}

std::array<uint8_t, 10> OpalysAudioProcessor::getModuleOrder() const noexcept
{
    std::array<uint8_t, 10> current;
    for (size_t i = 0; i < 10; ++i)
        current[i] = moduleOrder[i].load(std::memory_order_acquire);
    return current;
}

void OpalysAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) noexcept
{
    juce::ScopedNoDenormals noDenormals;

    if (panicRequested.exchange(false, std::memory_order_acquire))
    {
        saturationFX.reset(); tapeFX.reset(); sparklesFX.reset();
        delayFX.reset(); reverbFX.reset(); widerFX.reset();
        enhancerFX.reset(); pitchFX.reset(); breakerFX.reset(); ricochetFX.reset();
        buffer.clear();
        return;
    }

    auto* bypassParam = apvts.getRawParameterValue("master_bypass");
    if (bypassParam != nullptr && *bypassParam > 0.5f)
        return;

    double currentPpq = 0.0;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (pos->getPpqPosition().hasValue())
                currentPpq = *pos->getPpqPosition();
        }
    }

    for (size_t slot = 0; slot < 10; ++slot)
    {
        const uint8_t moduleId = moduleOrder[slot].load(std::memory_order_acquire);
        if (moduleId > 9) continue;

        switch (moduleId)
        {
            case 0: saturationFX.processBlock(buffer); break;
            case 1: tapeFX.processBlock(buffer); break;
            case 2: sparklesFX.processBlock(buffer); break;
            case 3: delayFX.processBlock(buffer); break;
            case 4: reverbFX.processBlock(buffer); break;
            case 5: widerFX.processBlock(buffer); break;
            case 6: enhancerFX.processBlock(buffer); break;
            case 7: pitchFX.processBlock(buffer); break;
            case 8: breakerFX.processBlock(buffer, currentPpq); break;
            case 9: ricochetFX.processBlock(buffer); break;
            default: break;
        }
    }

    auto* gainParam = apvts.getRawParameterValue("master_gain");
    const float masterGain = juce::Decibels::decibelsToGain(gainParam != nullptr ? gainParam->load() : 0.0f);
    buffer.applyGain(masterGain);
}

juce::AudioProcessorEditor* OpalysAudioProcessor::createEditor()
{
    return new OpalysAudioProcessorEditor(*this);
}

void OpalysAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    if (state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        if (xml != nullptr)
        {
            copyXmlToBinary(*xml, destData);
        }
    }
}

void OpalysAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (sizeInBytes <= 0 || data == nullptr)
        return;

    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml(*xmlState);
        if (tree.isValid())
        {
            apvts.replaceState(tree);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpalysAudioProcessor();
}
