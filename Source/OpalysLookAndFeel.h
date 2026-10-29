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
            setColour(juce::Slider::thumbColourId, juce::Colour(0xff0f172a));
            setColour(juce::Slider::trackColourId, juce::Colour(0xffcbd5e1));
        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override
        {
            auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                 static_cast<float>(width), static_cast<float>(height)).reduced(6.0f);
            auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
            auto center = bounds.getCentre();
            auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            // Fondo de la perilla
            g.setColour(juce::Colour(0xfff8fafc));
            g.fillEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);

            // Contorno
            g.setColour(juce::Colour(0xffcbd5e1));
            g.drawEllipse(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

            // Indicador de rotación
            juce::Path p;
            auto pointerLength = radius * 0.65f;
            auto pointerThickness = 2.5f;
            p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
            p.applyTransform(juce::AffineTransform::rotation(angle).translated(center.x, center.y));

            g.setColour(juce::Colour(0xff2563eb));
            g.fillPath(p);
        }
    };
}
