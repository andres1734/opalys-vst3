#include "PluginProcessor.h"
#include "PluginEditor.h"

void OpalysAudioProcessorEditor::ModuleTabButton::paintButton(juce::Graphics& g,
                                                             bool shouldDrawButtonAsHighlighted,
                                                             bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const float corner = 8.0f;
    juce::Colour bg = isCurrentActive ? juce::Colour(0xff2563eb)
                                      : (shouldDrawButtonAsHighlighted ? juce::Colour(0xfff1f5f9) : juce::Colour(0xffffffff));
    if (shouldDrawButtonAsDown) bg = bg.darker(0.06f);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, corner);

    g.setColour(isCurrentActive ? juce::Colour(0xff1d4ed8) : juce::Colour(0xffcbd5e1));
    g.drawRoundedRectangle(bounds, corner, 1.0f);

    auto ledArea = bounds.removeFromRight(14.0f).removeFromTop(14.0f).translated(-4.0f, 4.0f);
    g.setColour(isEffectEnabled ? (isCurrentActive ? juce::Colour(0xffffffff) : juce::Colour(0xff10b981))
                                : juce::Colour(0xff94a3b8));
    g.fillEllipse(ledArea);

    g.setColour(isCurrentActive ? juce::Colour(0xffffffff) : juce::Colour(0xff0f172a));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawFittedText(getName(), getLocalBounds().reduced(6, 2), juce::Justification::centredLeft, 2);
}

OpalysAudioProcessorEditor::OpalysAudioProcessorEditor(OpalysAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(1180, 760);

    brandLabel.setText("O", juce::dontSendNotification);
    brandLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    brandLabel.setColour(juce::Label::textColourId, juce::Colour(0xff2563eb));
    addAndMakeVisible(brandLabel);

    pluginTitleLabel.setText("Opalys - Motor Multiefectos", juce::dontSendNotification);
    pluginTitleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    addAndMakeVisible(pluginTitleLabel);

    addAndMakeVisible(powerButton);
    addAndMakeVisible(prevPresetBtn);
    addAndMakeVisible(nextPresetBtn);
    presetSelector.addItem("Sin preset - Estado inicial", 1);
    presetSelector.addItem("Catedral Pastel · Shimmer Angelical", 2);
    presetSelector.addItem("Cinta Nostalgia 1984 · Lo-Fi Calido", 3);
    presetSelector.addItem("Pulso Ritmico FL · Breaker & Ricochet", 4);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(presetSelector);
    addAndMakeVisible(settingsBtn);

    auto setupMacroSlider = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
        addAndMakeVisible(s);
    };

    setupMacroSlider(macroCuerpo); setupMacroSlider(macroAire); setupMacroSlider(macroSuciedad);
    setupMacroSlider(macroEspacio); setupMacroSlider(macroEco); setupMacroSlider(macroMovimiento);
    setupMacroSlider(masterLowCut); setupMacroSlider(masterHighCut); setupMacroSlider(masterVolume);

    // El boton de bypass debe ser tipo "toggle" para que el attachment funcione
    bypassButton.setClickingTogglesState(true);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(panicButton);
    panicButton.onClick = [this] { audioProcessor.triggerPanic(); };

    // Enlaces seguros (Attachments)
    attCuerpo     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_cuerpo", macroCuerpo);
    attAire       = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_aire", macroAire);
    attSuciedad   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_suciedad", macroSuciedad);
    attEspacio    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_espacio", macroEspacio);
    attEco        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_eco", macroEco);
    attMovimiento = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "macro_movimiento", macroMovimiento);

    attLowCut     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "master_lowcut", masterLowCut);
    attHighCut    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "master_highcut", masterHighCut);
    attVolume     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "master_gain", masterVolume);
    attBypass     = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "master_bypass", bypassButton);

    const juce::String moduleNames[10] = {
        "01 Sat", "02 Tape", "03 Spark", "04 Delay", "05 Reverb",
        "06 Wider", "07 Enhan", "08 Pitch", "09 Break", "10 Rico"
    };

    for (int i = 0; i < 10; ++i)
    {
        rackButtons[i] = std::make_unique<ModuleTabButton>(moduleNames[i], i);
        rackButtons[i]->isCurrentActive = (i == 0);
        rackButtons[i]->onClick = [this, i]() {
            selectedModuleIndex = i;
            for (int k = 0; k < 10; ++k)
                rackButtons[k]->isCurrentActive = (k == i);
            repaint();
        };
        addAndMakeVisible(*rackButtons[i]);
    }

    startTimerHz(60);
}

OpalysAudioProcessorEditor::~OpalysAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void OpalysAudioProcessorEditor::timerCallback()
{
    plasmaPhase += 0.02f;
    repaint();
}

void OpalysAudioProcessorEditor::drawPlasmaVisualizer(juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Graphics::ScopedSaveState state(g);
    juce::Path clipPath;
    clipPath.addEllipse(area);
    g.reduceClipRegion(clipPath);

    const float cx = area.getCentreX();
    const float cy = area.getCentreY();
    const float w  = area.getWidth();
    const float h  = area.getHeight();

    juce::ColourGradient grad(
        juce::Colour(0xfffce7f3), cx + std::sin(plasmaPhase * 0.9f) * 60.0f, cy,
        juce::Colour(0xffcffafe), cx + std::cos(plasmaPhase * 1.1f) * 60.0f, cy, true);
    grad.addColour(0.45, juce::Colour(0xfffef08a));
    grad.addColour(0.85, juce::Colour(0xfffed7aa));
    g.setGradientFill(grad);
    g.fillEllipse(area);

    g.setColour(juce::Colour(0x351e293b));
    juce::Path wave;
    for (float x = area.getX(); x <= area.getRight(); x += 4.0f)
    {
        const float normX = (x - area.getX()) / w;
        const float y = cy + std::sin(normX * 12.0f + plasmaPhase * 2.5f) * (h * 0.15f) * std::sin(normX * juce::MathConstants<float>::pi);
        if (x == area.getX()) wave.startNewSubPath(x, y);
        else wave.lineTo(x, y);
    }
    g.strokePath(wave, juce::PathStrokeType(2.0f));
    g.setColour(juce::Colour(0x5064748b));
    g.drawEllipse(area.reduced(0.5f), 1.5f);
}

void OpalysAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(findColour(juce::ResizableWindow::backgroundColourId));

    auto drawCard = [&g](juce::Rectangle<float> bounds) {
        g.setColour(juce::Colour(0x150f172a));
        g.fillRoundedRectangle(bounds.translated(0.0f, 2.0f), 10.0f);
        g.setColour(juce::Colour(0xffffffff));
        g.fillRoundedRectangle(bounds, 10.0f);
        g.setColour(juce::Colour(0xffe2e8f0));
        g.drawRoundedRectangle(bounds, 10.0f, 1.0f);
    };

    drawCard(juce::Rectangle<float>(16.0f, 60.0f, getWidth() - 32.0f, 230.0f));
    const auto ellipseArea = juce::Rectangle<float>(getWidth() * 0.5f - 240.0f, 75.0f, 480.0f, 175.0f);
    drawPlasmaVisualizer(g, ellipseArea);
    drawCard(juce::Rectangle<float>(16.0f, 305.0f, getWidth() - 32.0f, 280.0f));
    drawCard(juce::Rectangle<float>(16.0f, 600.0f, getWidth() - 32.0f, 145.0f));
}

void OpalysAudioProcessorEditor::resized()
{
    auto total = getLocalBounds().reduced(16);
    auto topBar = total.removeFromTop(40);
    brandLabel.setBounds(topBar.removeFromLeft(30));
    pluginTitleLabel.setBounds(topBar.removeFromLeft(240));
    powerButton.setBounds(topBar.removeFromLeft(90).reduced(2));

    settingsBtn.setBounds(topBar.removeFromRight(80).reduced(2));
    nextPresetBtn.setBounds(topBar.removeFromRight(30).reduced(2));
    presetSelector.setBounds(topBar.removeFromRight(260).reduced(2));
    prevPresetBtn.setBounds(topBar.removeFromRight(30).reduced(2));

    const int macroW = 75;
    const int macroY = 120;
    macroCuerpo.setBounds(35, macroY, macroW, 85);
    macroAire.setBounds(120, macroY, macroW, 85);
    macroSuciedad.setBounds(205, macroY, macroW, 85);

    const int rightStartX = getWidth() - 295;
    macroEspacio.setBounds(rightStartX, macroY, macroW, 85);
    macroEco.setBounds(rightStartX + 85, macroY, macroW, 85);
    macroMovimiento.setBounds(rightStartX + 170, macroY, macroW, 85);

    auto bottomArea = total.removeFromBottom(130);
    auto rackRow = bottomArea.removeFromTop(52);
    const int tabWidth = (rackRow.getWidth() - 9 * 6) / 10;
    for (int i = 0; i < 10; ++i)
    {
        rackButtons[i]->setBounds(rackRow.removeFromLeft(tabWidth));
        rackRow.removeFromLeft(6);
    }

    bottomArea.removeFromTop(8);
    auto masterRow = bottomArea;
    masterLowCut.setBounds(masterRow.removeFromLeft(75));
    masterHighCut.setBounds(masterRow.removeFromLeft(75));
    masterVolume.setBounds(masterRow.removeFromLeft(75));

    panicButton.setBounds(masterRow.removeFromRight(85).reduced(4));
    bypassButton.setBounds(masterRow.removeFromRight(85).reduced(4));
}
