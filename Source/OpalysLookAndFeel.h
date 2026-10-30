#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace opalys::gui
{
    class OpalysLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        OpalysLookAndFeel()
        {
            setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff2563eb));
            setColour(juce::Slider::thumbColourId,            juce::Colour(0xff0f172a));
            setColour(juce::Slider::trackColourId,            juce::Colour(0xffcbd5e1));

            // Texto de valores debajo de las perillas
            setColour(juce::Slider::textBoxTextColourId,       juce::Colour(0xff0f172a));
            setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentWhite);
            setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentWhite);
            setColour(juce::Slider::textBoxHighlightColourId,  juce::Colour(0x402563eb));

            setColour(juce::Label::textColourId, juce::Colour(0xff0f172a));
        }

        // Perilla con relieve: aro de valor, cuerpo con degradado, sombra suave y punto indicador.
        // El nombre de la perilla (slider.getName()) se dibuja debajo del dial.
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override
        {
            auto area = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height));

            const juce::String name = slider.getName();
            const float nameH = name.isNotEmpty() ? juce::jlimit(12.0f, 20.0f, area.getHeight() * 0.16f) : 0.0f;
            auto nameArea = area.removeFromBottom(nameH);

            auto knobArea = area.reduced(4.0f);
            const float radius = juce::jmin(knobArea.getWidth(), knobArea.getHeight()) * 0.5f;
            const auto centre = knobArea.getCentre();
            const float cx = centre.x;
            const float cy = centre.y;
            const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // Aro de fondo y aro de valor
            const float ringT = juce::jmax(2.5f, radius * 0.09f);
            const float ringR = radius - ringT * 0.5f;

            juce::Path track;
            track.addCentredArc(cx, cy, ringR, ringR, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
            g.setColour(juce::Colour(0xffd5d9e1));
            g.strokePath(track, juce::PathStrokeType(ringT, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path value;
            value.addCentredArc(cx, cy, ringR, ringR, 0.0f, rotaryStartAngle, angle, true);
            g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
            g.strokePath(value, juce::PathStrokeType(ringT, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            // Cuerpo de la perilla
            const float bodyR = radius - ringT * 2.2f;
            const auto bodyRect = juce::Rectangle<float>(bodyR * 2.0f, bodyR * 2.0f).withCentre(centre);

            for (int i = 0; i < 4; ++i)
            {
                g.setColour(juce::Colour(0x12000000));
                g.fillEllipse(bodyRect.expanded(static_cast<float>(i) * 1.2f)
                                      .translated(0.0f, 2.0f + static_cast<float>(i)));
            }

            g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffffff), cx - bodyR * 0.4f, cy - bodyR * 0.6f,
                                                   juce::Colour(0xffdfe3ea), cx + bodyR * 0.5f, cy + bodyR * 0.8f, false));
            g.fillEllipse(bodyRect);

            g.setColour(juce::Colour(0xffc9cfd8));
            g.drawEllipse(bodyRect, 1.0f);

            // Punto indicador
            const float dotR = juce::jmax(2.2f, bodyR * 0.09f);
            const float dotDist = bodyR * 0.68f;
            const float dx = cx + dotDist * std::sin(angle);
            const float dy = cy - dotDist * std::cos(angle);
            g.setColour(juce::Colour(0xff0f172a));
            g.fillEllipse(dx - dotR, dy - dotR, dotR * 2.0f, dotR * 2.0f);

            // Nombre
            if (name.isNotEmpty())
            {
                g.setColour(juce::Colour(0xff374151));
                g.setFont(juce::FontOptions(juce::jlimit(9.0f, 13.0f, nameArea.getHeight() * 0.75f), juce::Font::bold));
                g.drawText(name, nameArea, juce::Justification::centred, false);
            }
        }
    };
}
