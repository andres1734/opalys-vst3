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

    brandLabel.setText("O", juce::dontSendNotification);
    brandLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    brandLabel.setColour(juce::Label::textColourId, juce::Colour(0xff2563eb));
    addAndMakeVisible(brandLabel);

    pluginTitleLabel.setText("Opalys v2 - Motor Multiefectos", juce::dontSendNotification);
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

    // Ventana redimensionable con proporcion fija (diseno base 1180x760)
    setResizable(true, true);
    setResizeLimits(650, 419, 1400, 902);
    getConstrainer()->setFixedAspectRatio(1180.0 / 760.0);

    // setSize llama a resized(), asi que va al final, cuando todo ya existe
    setSize(900, 580);
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

    const float s = static_cast<float>(getWidth()) / 1180.0f;   // factor de escala

    auto drawCard = [&g, s](juce::Rectangle<float> bounds) {
        g.setColour(juce::Colour(0x150f172a));
        g.fillRoundedRectangle(bounds.translated(0.0f, 2.0f * s), 10.0f * s);
        g.setColour(juce::Colour(0xffffffff));
        g.fillRoundedRectangle(bounds, 10.0f * s);
        g.setColour(juce::Colour(0xffe2e8f0));
        g.drawRoundedRectangle(bounds, 10.0f * s, 1.0f);
    };

    const float w = static_cast<float>(getWidth());

    drawCard(juce::Rectangle<float>(16.0f * s, 60.0f * s, w - 32.0f * s, 230.0f * s));
    const auto ellipseArea = juce::Rectangle<float>(w * 0.5f - 240.0f * s, 75.0f * s, 480.0f * s, 175.0f * s);
    drawPlasmaVisualizer(g, ellipseArea);
    drawCard(juce::Rectangle<float>(16.0f * s, 305.0f * s, w - 32.0f * s, 280.0f * s));
    drawCard(juce::Rectangle<float>(16.0f * s, 600.0f * s, w - 32.0f * s, 145.0f * s));
}

void OpalysAudioProcessorEditor::resized()
{
    // Seguridad: si los botones del rack aun no existen, no hacer nada
    if (rackButtons[9] == nullptr)
        return;

    const float s = static_cast<float>(getWidth()) / 1180.0f;   // factor de escala
    auto S = [s](float v) { return juce::roundToInt(v * s); };

    auto total = getLocalBounds().reduced(S(16.0f));
    auto topBar = total.removeFromTop(S(40.0f));
    brandLabel.setBounds(topBar.removeFromLeft(S(30.0f)));
    pluginTitleLabel.setBounds(topBar.removeFromLeft(S(240.0f)));
    powerButton.setBounds(topBar.removeFromLeft(S(90.0f)).reduced(2));

    settingsBtn.setBounds(topBar.removeFromRight(S(80.0f)).reduced(2));
    nextPresetBtn.setBounds(topBar.removeFromRight(S(40.0f)).reduced(2));
    presetSelector.setBounds(topBar.removeFromRight(S(260.0f)).reduced(2));
    prevPresetBtn.setBounds(topBar.removeFromRight(S(40.0f)).reduced(2));

    const int macroW = S(75.0f);
    const int macroH = S(85.0f);
    const int macroY = S(120.0f);
    macroCuerpo.setBounds(S(35.0f),  macroY, macroW, macroH);
    macroAire.setBounds(S(120.0f),   macroY, macroW, macroH);
    macroSuciedad.setBounds(S(205.0f), macroY, macroW, macroH);

    const int rightStartX = getWidth() - S(295.0f);
    macroEspacio.setBounds(rightStartX,                 macroY, macroW, macroH);
    macroEco.setBounds(rightStartX + S(85.0f),          macroY, macroW, macroH);
    macroMovimiento.setBounds(rightStartX + S(170.0f),  macroY, macroW, macroH);

    auto bottomArea = total.removeFromBottom(S(130.0f));
    auto rackRow = bottomArea.removeFromTop(S(52.0f));
    const int gap = S(6.0f);
    const int tabWidth = (rackRow.getWidth() - 9 * gap) / 10;
    for (int i = 0; i < 10; ++i)
    {
        rackButtons[i]->setBounds(rackRow.removeFromLeft(tabWidth));
        rackRow.removeFromLeft(gap);
    }

    bottomArea.removeFromTop(S(8.0f));
    auto masterRow = bottomArea;
    masterLowCut.setBounds(masterRow.removeFromLeft(macroW));
    masterHighCut.setBounds(masterRow.removeFromLeft(macroW));
    masterVolume.setBounds(masterRow.removeFromLeft(macroW));

    panicButton.setBounds(masterRow.removeFromRight(S(85.0f)).reduced(4));
    bypassButton.setBounds(masterRow.removeFromRight(S(85.0f)).reduced(4));
}
