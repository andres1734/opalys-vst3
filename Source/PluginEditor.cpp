#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Diseno base: 1180 x 655. Todo se escala con este factor.
    constexpr float kDesignW = 1180.0f;
    constexpr float kDesignH = 655.0f;

    float scaleFor(int w, int h)
    {
        return juce::jmax(0.1f, juce::jmin(static_cast<float>(w) / kDesignW,
                                           static_cast<float>(h) / kDesignH));
    }

    juce::Rectangle<float> plasmaAreaFor(float s)
    {
        return juce::Rectangle<float>(425.0f * s, 128.0f * s, 330.0f * s, 220.0f * s);
    }

    const char* const kModuleSubtitles[10] = {
        "calidez + agudos", "wow \xC2\xB7 lo-fi", "textura dispersa", "eco r\xC3\xADtmico", "espacio y cola",
        "imagen est\xC3\xA9reo", "realce din\xC3\xA1mico", "cambio de tono", "tiempo y volumen", "repeticiones"
    };

    const juce::uint32 kModuleTags[10] = {
        0xfff59e0b, 0xff14b8a6, 0xff8b5cf6, 0xff3b82f6, 0xffec4899,
        0xff22c55e, 0xffef4444, 0xffa855f7, 0xff0ea5e9, 0xfffbbf24
    };
}

void OpalysAudioProcessorEditor::ModuleTabButton::paintButton(juce::Graphics& g,
                                                             bool shouldDrawButtonAsHighlighted,
                                                             bool shouldDrawButtonAsDown)
{
    const float sc = juce::jmax(0.5f, juce::jmin(static_cast<float>(getWidth()) / 105.8f,
                                                  static_cast<float>(getHeight()) / 100.0f));
    auto bounds = getLocalBounds().toFloat().reduced(1.5f);
    const float corner = 10.0f * sc;
    const int idx = juce::jlimit(0, 9, slotIndex);

    // Sombra
    g.setColour(juce::Colour(0x14000000));
    g.fillRoundedRectangle(bounds.translated(0.0f, 2.0f * sc), corner);

    // Fondo
    juce::Colour top    = isCurrentActive ? juce::Colour(0xffeef3ff) : juce::Colour(0xfffbfcfd);
    juce::Colour bottom = isCurrentActive ? juce::Colour(0xffdbe6ff) : juce::Colour(0xffedeff3);
    if (shouldDrawButtonAsHighlighted) { top = top.brighter(0.03f); }
    if (shouldDrawButtonAsDown)        { top = top.darker(0.05f); bottom = bottom.darker(0.05f); }

    g.setGradientFill(juce::ColourGradient(top, 0.0f, bounds.getY(), bottom, 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle(bounds, corner);

    g.setColour(isCurrentActive ? juce::Colour(0xff2563eb) : juce::Colour(0xffd5d9e0));
    g.drawRoundedRectangle(bounds, corner, isCurrentActive ? 2.0f : 1.0f);

    // LED de encendido (izquierda)
    const float led = 8.0f * sc;
    g.setColour(isEffectEnabled ? juce::Colour(0xff10b981) : juce::Colour(0xff94a3b8));
    g.fillEllipse(bounds.getX() + 10.0f * sc, bounds.getY() + 10.0f * sc, led, led);

    // Etiqueta de color (derecha)
    const float tag = 16.0f * sc;
    const auto tagRect = juce::Rectangle<float>(bounds.getRight() - 10.0f * sc - tag,
                                                bounds.getY() + 8.0f * sc, tag, tag);
    g.setColour(juce::Colour(kModuleTags[idx]).withAlpha(0.9f));
    g.fillRoundedRectangle(tagRect, 4.0f * sc);

    // Nombre y subtitulo
    auto textArea = bounds.reduced(10.0f * sc, 0.0f);

    g.setColour(juce::Colour(0xff0f172a));
    g.setFont(juce::FontOptions(juce::jmax(8.0f, 11.5f * sc), juce::Font::bold));
    g.drawText(getName(),
               textArea.withTop(bounds.getY() + bounds.getHeight() * 0.50f).withHeight(18.0f * sc),
               juce::Justification::centredLeft, true);

    g.setColour(juce::Colour(0xff94a3b8));
    g.setFont(juce::FontOptions(juce::jmax(7.0f, 9.0f * sc)));
    g.drawText(juce::String::fromUTF8(kModuleSubtitles[idx]),
               textArea.withTop(bounds.getY() + bounds.getHeight() * 0.73f).withHeight(14.0f * sc),
               juce::Justification::centredLeft, true);
}

OpalysAudioProcessorEditor::OpalysAudioProcessorEditor(OpalysAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&lookAndFeel);

    // Cabecera
    brandLabel.setText("OPALYS", juce::dontSendNotification);
    brandLabel.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    brandLabel.setColour(juce::Label::textColourId, juce::Colour(0xff0f172a));
    addAndMakeVisible(brandLabel);

    pluginTitleLabel.setText("MOTOR MULTIEFECTOS", juce::dontSendNotification);
    pluginTitleLabel.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    pluginTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(pluginTitleLabel);

    // Estilo de botones de texto
    auto styleButton = [](juce::TextButton& b, juce::Colour bg, juce::Colour txt)
    {
        b.setColour(juce::TextButton::buttonColourId,   bg);
        b.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff2563eb));
        b.setColour(juce::TextButton::textColourOffId,  txt);
        b.setColour(juce::TextButton::textColourOnId,   juce::Colours::white);
    };

    const juce::Colour lightBtn(0xfff8fafc);
    const juce::Colour darkTxt(0xff0f172a);

    styleButton(powerButton,   lightBtn, darkTxt);
    styleButton(prevPresetBtn, lightBtn, darkTxt);
    styleButton(nextPresetBtn, lightBtn, darkTxt);
    styleButton(settingsBtn,   lightBtn, darkTxt);
    styleButton(bypassButton,  juce::Colour(0xffe5e7eb), juce::Colour(0xff475569));
    styleButton(panicButton,   juce::Colour(0xfffbd5d5), juce::Colour(0xffb91c1c));

    bypassButton.setButtonText("BYPASS");
    panicButton.setButtonText(juce::String::fromUTF8("P\xC3\x81NICO"));

    addAndMakeVisible(powerButton);
    addAndMakeVisible(prevPresetBtn);
    addAndMakeVisible(nextPresetBtn);

    presetSelector.addItem("Sin preset - Estado inicial", 1);
    presetSelector.addItem(juce::String::fromUTF8("Catedral Pastel \xC2\xB7 Shimmer Angelical"), 2);
    presetSelector.addItem(juce::String::fromUTF8("Cinta Nostalgia 1984 \xC2\xB7 Lo-Fi Calido"), 3);
    presetSelector.addItem(juce::String::fromUTF8("Pulso Ritmico FL \xC2\xB7 Breaker & Ricochet"), 4);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.setJustificationType(juce::Justification::centred);
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xfff8fafc));
    presetSelector.setColour(juce::ComboBox::textColourId,       darkTxt);
    presetSelector.setColour(juce::ComboBox::outlineColourId,    juce::Colour(0xffcbd5e1));
    presetSelector.setColour(juce::ComboBox::arrowColourId,      juce::Colour(0xff64748b));
    addAndMakeVisible(presetSelector);
    addAndMakeVisible(settingsBtn);

    // Perillas
    auto setupSlider = [this](juce::Slider& s, const juce::String& name)
    {
        s.setName(name);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);

        s.setColour(juce::Slider::textBoxTextColourId,       juce::Colour(0xff0f172a));
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentWhite);
        s.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentWhite);
        s.setColour(juce::Slider::textBoxHighlightColourId,  juce::Colour(0x402563eb));

        addAndMakeVisible(s);
    };

    setupSlider(macroCuerpo,     "Cuerpo");
    setupSlider(macroAire,       "Aire");
    setupSlider(macroSuciedad,   "Suciedad");
    setupSlider(macroEspacio,    "Espacio");
    setupSlider(macroEco,        "Eco");
    setupSlider(macroMovimiento, "Movimiento");
    setupSlider(masterLowCut,    "Graves");
    setupSlider(masterHighCut,   "Agudos");
    setupSlider(masterVolume,    "Volumen");

    // Boton de bypass: debe ser toggle para que el attachment funcione
    bypassButton.setClickingTogglesState(true);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(panicButton);
    panicButton.onClick = [this] { audioProcessor.triggerPanic(); };

    // Attachments
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

    // Formato del texto de valor (despues de los attachments, para que no lo sobrescriban)
    auto parseNumber = [](const juce::String& t)
    {
        double v = t.retainCharacters("0123456789.-").getDoubleValue();
        if (t.containsIgnoreCase("k"))
            v *= 1000.0;
        return v;
    };

    for (auto* sl : { &macroCuerpo, &macroAire, &macroSuciedad, &macroEspacio, &macroEco, &macroMovimiento })
    {
        sl->textFromValueFunction = [](double v)
        {
            const int i = juce::roundToInt(v);
            return juce::String(i > 0 ? "+" : "") + juce::String(i) + "%";
        };
        sl->valueFromTextFunction = parseNumber;
        sl->updateText();
    }

    masterLowCut.textFromValueFunction = [](double v) { return juce::String(juce::roundToInt(v)) + " Hz"; };
    masterLowCut.valueFromTextFunction = parseNumber;
    masterLowCut.updateText();

    masterHighCut.textFromValueFunction = [](double v)
    {
        return v >= 1000.0 ? juce::String(v / 1000.0, 1) + " kHz"
                           : juce::String(juce::roundToInt(v)) + " Hz";
    };
    masterHighCut.valueFromTextFunction = parseNumber;
    masterHighCut.updateText();

    masterVolume.textFromValueFunction = [](double v) { return juce::String(v, 1) + " dB"; };
    masterVolume.valueFromTextFunction = parseNumber;
    masterVolume.updateText();

    // Tarjetas de modulos
    const juce::String moduleNames[10] = {
        juce::String::fromUTF8("SATURACI\xC3\x93N"), "TAPE + NOISE", "SPARKLES", "DELAY", "REVERB",
        "WIDER", "ENHANCER", "PITCH", "BREAKER", "RICOCHET"
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

    // Ventana redimensionable con proporcion fija
    setResizable(true, true);
    setResizeLimits(650, 361, 1400, 777);
    getConstrainer()->setFixedAspectRatio(static_cast<double>(kDesignW) / static_cast<double>(kDesignH));

    // setSize llama a resized(), asi que va al final, cuando todo ya existe
    setSize(900, 500);
    startTimerHz(60);
}

OpalysAudioProcessorEditor::~OpalysAudioProcessorEditor()
{
    stopTimer();

    // Soltar los attachments primero, antes de que se destruyan los controles
    attCuerpo.reset(); attAire.reset(); attSuciedad.reset();
    attEspacio.reset(); attEco.reset(); attMovimiento.reset();
    attLowCut.reset(); attHighCut.reset(); attVolume.reset();
    attBypass.reset();

    setLookAndFeel(nullptr);
}

void OpalysAudioProcessorEditor::timerCallback()
{
    plasmaPhase += 0.02f;

    // Solo se redibuja el ovalo, no toda la ventana
    const float s = scaleFor(getWidth(), getHeight());
    repaint(plasmaAreaFor(s).getSmallestIntegerContainer().expanded(2));
}

void OpalysAudioProcessorEditor::drawPlasmaVisualizer(juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Graphics::ScopedSaveState state(g);
    juce::Path clipPath;
    clipPath.addEllipse(area);
    g.reduceClipRegion(clipPath);

    const float k  = area.getWidth() / 330.0f;
    const float cx = area.getCentreX();
    const float cy = area.getCentreY();
    const float w  = area.getWidth();
    const float h  = area.getHeight();

    juce::ColourGradient grad(
        juce::Colour(0xfffce7f3), cx + std::sin(plasmaPhase * 0.9f) * 60.0f * k, cy,
        juce::Colour(0xffcffafe), cx + std::cos(plasmaPhase * 1.1f) * 60.0f * k, cy, true);
    grad.addColour(0.45, juce::Colour(0xfffef08a));
    grad.addColour(0.85, juce::Colour(0xfffed7aa));
    g.setGradientFill(grad);
    g.fillEllipse(area);

    g.setColour(juce::Colour(0x351e293b));
    juce::Path wave;
    const float step = juce::jmax(2.0f, 4.0f * k);
    for (float x = area.getX(); x <= area.getRight(); x += step)
    {
        const float normX = (x - area.getX()) / w;
        const float y = cy + std::sin(normX * 12.0f + plasmaPhase * 2.5f) * (h * 0.15f) * std::sin(normX * juce::MathConstants<float>::pi);
        if (x == area.getX()) wave.startNewSubPath(x, y);
        else wave.lineTo(x, y);
    }
    g.strokePath(wave, juce::PathStrokeType(2.0f * k));
    g.setColour(juce::Colour(0x5064748b));
    g.drawEllipse(area.reduced(0.5f), 1.5f);
}

void OpalysAudioProcessorEditor::paint(juce::Graphics& g)
{
    const float s = scaleFor(getWidth(), getHeight());

    // Fondo
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff4f5f8), 0.0f, 0.0f,
                                           juce::Colour(0xffe1e4ea), 0.0f, static_cast<float>(getHeight()), false));
    g.fillAll();

    auto drawCard = [&g, s](juce::Rectangle<float> r)
    {
        const float cr = 14.0f * s;
        for (int i = 0; i < 4; ++i)
        {
            const float fi = static_cast<float>(i);
            g.setColour(juce::Colour(0x0d1e293b));
            g.fillRoundedRectangle(r.expanded(fi * 1.5f * s).translated(0.0f, (2.0f + fi * 1.5f) * s), cr + fi * 1.5f * s);
        }
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xfffbfcfd), 0.0f, r.getY(),
                                               juce::Colour(0xffeceef3), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle(r, cr);
        g.setColour(juce::Colour(0xffffffff));
        g.drawRoundedRectangle(r.reduced(1.0f), cr, 1.0f);
        g.setColour(juce::Colour(0xffd3d8e0));
        g.drawRoundedRectangle(r, cr, 1.0f);
    };

    // Paneles de macros y franja master
    drawCard(juce::Rectangle<float>( 16.0f * s,  78.0f * s,  395.0f * s, 320.0f * s));
    drawCard(juce::Rectangle<float>(769.0f * s,  78.0f * s,  395.0f * s, 320.0f * s));
    drawCard(juce::Rectangle<float>( 16.0f * s, 526.0f * s, 1148.0f * s, 110.0f * s));

    // Titulo del panel de macros
    g.setColour(juce::Colour(0xff334155));
    g.setFont(juce::FontOptions(juce::jmax(9.0f, 13.0f * s), juce::Font::bold));
    g.drawText("MACROS", juce::Rectangle<float>(36.0f * s, 90.0f * s, 200.0f * s, 18.0f * s),
               juce::Justification::centredLeft, false);
    g.setColour(juce::Colour(0xff94a3b8));
    g.setFont(juce::FontOptions(juce::jmax(8.0f, 10.0f * s)));
    g.drawText("tono general del sonido", juce::Rectangle<float>(36.0f * s, 108.0f * s, 250.0f * s, 14.0f * s),
               juce::Justification::centredLeft, false);

    // Ovalo central
    drawPlasmaVisualizer(g, plasmaAreaFor(s));

    // Franja master: etiqueta vertical y nombre
    {
        juce::Graphics::ScopedSaveState state(g);
        g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi,
                                                       34.0f * s, 581.0f * s));
        g.setColour(juce::Colour(0xff94a3b8));
        g.setFont(juce::FontOptions(juce::jmax(7.0f, 9.0f * s), juce::Font::bold));
        g.drawText("MASTER", juce::Rectangle<float>(34.0f * s - 40.0f * s, 581.0f * s - 8.0f * s, 80.0f * s, 16.0f * s),
                   juce::Justification::centred, false);
    }

    g.setColour(juce::Colour(0xff0f172a));
    g.setFont(juce::FontOptions(juce::jmax(10.0f, 22.0f * s), juce::Font::bold));
    g.drawText("M I D I L A T I N O", juce::Rectangle<float>(80.0f * s, 566.0f * s, 400.0f * s, 30.0f * s),
               juce::Justification::centredLeft, false);
}

void OpalysAudioProcessorEditor::resized()
{
    // Seguridad: si los botones del rack aun no existen, no hacer nada
    if (rackButtons[9] == nullptr)
        return;

    const float s = scaleFor(getWidth(), getHeight());
    auto B = [s](float x, float y, float w, float h)
    {
        return juce::Rectangle<int>(juce::roundToInt(x * s), juce::roundToInt(y * s),
                                    juce::roundToInt(w * s), juce::roundToInt(h * s));
    };

    // Cabecera
    brandLabel.setBounds(B(20.0f, 10.0f, 200.0f, 30.0f));
    pluginTitleLabel.setBounds(B(20.0f, 38.0f, 200.0f, 16.0f));
    powerButton.setBounds(B(236.0f, 18.0f, 90.0f, 30.0f));

    prevPresetBtn.setBounds(B(430.0f, 18.0f, 36.0f, 30.0f));
    presetSelector.setBounds(B(470.0f, 18.0f, 240.0f, 30.0f));
    nextPresetBtn.setBounds(B(714.0f, 18.0f, 36.0f, 30.0f));
    settingsBtn.setBounds(B(1074.0f, 18.0f, 90.0f, 30.0f));

    // Macros (izquierda y derecha)
    const float my = 150.0f, mw = 110.0f, mh = 170.0f;
    macroCuerpo.setBounds(B(35.0f,  my, mw, mh));
    macroAire.setBounds(B(158.0f,   my, mw, mh));
    macroSuciedad.setBounds(B(281.0f, my, mw, mh));

    macroEspacio.setBounds(B(788.0f,    my, mw, mh));
    macroEco.setBounds(B(911.0f,        my, mw, mh));
    macroMovimiento.setBounds(B(1034.0f, my, mw, mh));

    // Tarjetas de modulos
    const float cardW = (1148.0f - 9.0f * 10.0f) / 10.0f;
    for (int i = 0; i < 10; ++i)
        rackButtons[i]->setBounds(B(16.0f + static_cast<float>(i) * (cardW + 10.0f), 412.0f, cardW, 100.0f));

    // Franja master
    masterLowCut.setBounds(B(640.0f, 527.0f, 95.0f, 108.0f));
    masterHighCut.setBounds(B(740.0f, 527.0f, 95.0f, 108.0f));
    masterVolume.setBounds(B(840.0f, 527.0f, 95.0f, 108.0f));

    bypassButton.setBounds(B(960.0f, 563.0f, 95.0f, 38.0f));
    panicButton.setBounds(B(1062.0f, 563.0f, 92.0f, 38.0f));
}
