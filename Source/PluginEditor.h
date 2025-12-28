#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
struct Sub808LookAndFeel : public juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height)
                          .reduced (8.0f);

        auto radius  = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto centre  = bounds.getCentre();
        auto rx      = centre.x - radius;
        auto ry      = centre.y - radius;
        auto rw      = radius * 2.0f;

        const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Colors
        const auto panel      = juce::Colour::fromRGB (18, 18, 22);
        const auto knobFace   = juce::Colour::fromRGB (38, 38, 48);
        const auto knobEdge   = juce::Colour::fromRGB (70, 70, 85);
        const auto arcBack    = juce::Colour::fromRGB (55, 55, 65);
        const auto arcValue   = juce::Colour::fromRGB (120, 200, 255); // adjust if you want
        const auto tickColour = juce::Colours::white.withAlpha (0.9f);

        // Knob shadow
        {
            juce::Path shadow;
            shadow.addEllipse (rx, ry + 2.0f, rw, rw);
            g.setColour (juce::Colours::black.withAlpha (0.35f));
            g.fillPath (shadow);
        }

        // Knob body
        {
            juce::ColourGradient grad (knobFace.brighter (0.2f), centre.x, centre.y - radius,
                                       knobFace.darker (0.3f),  centre.x, centre.y + radius, false);
            g.setGradientFill (grad);
            g.fillEllipse (rx, ry, rw, rw);

            g.setColour (knobEdge);
            g.drawEllipse (rx, ry, rw, rw, 1.5f);
        }

        // Arc background + value arc
        {
            juce::Path arc;
            arc.addCentredArc (centre.x, centre.y, radius + 6.0f, radius + 6.0f,
                               0.0f, rotaryStartAngle, rotaryEndAngle, true);

            g.setColour (arcBack);
            g.strokePath (arc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

            juce::Path valueArc;
            valueArc.addCentredArc (centre.x, centre.y, radius + 6.0f, radius + 6.0f,
                                    0.0f, rotaryStartAngle, angle, true);

            g.setColour (arcValue);
            g.strokePath (valueArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Pointer / tick
        {
            juce::Path p;
            const float pointerLen   = radius * 0.62f;
            const float pointerThick = 2.5f;

            p.addRoundedRectangle (-pointerThick * 0.5f, -pointerLen, pointerThick, pointerLen,
                                   pointerThick * 0.5f);

            g.setColour (tickColour);
            g.fillPath (p, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
        }

        // Optional: center cap
        {
            g.setColour (panel.withAlpha (0.7f));
            g.fillEllipse (centre.x - 5.0f, centre.y - 5.0f, 10.0f, 10.0f);
        }
    }
};

class Sub808AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    Sub808AudioProcessorEditor (Sub808AudioProcessor&);
    ~Sub808AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Sub808AudioProcessor& audioProcessor;

    juce::Slider gainSlider, attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Slider pitchSlider, glideSlider, driveSlider, colorSlider, toneSlider;
    juce::Label  gainLabel,  attackLabel,  decayLabel,  sustainLabel,  releaseLabel;
    juce::Label  pitchLabel, glideLabel, driveLabel, colorLabel, toneLabel;
    Sub808LookAndFeel lnf;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> gainAttach, attackAttach, decayAttach, sustainAttach, releaseAttach;
    std::unique_ptr<Attachment> pitchAttach, glideAttach, driveAttach, colorAttach, toneAttach;

    // Presets UI
    juce::ComboBox presetBox;

    // Simple preset structure and storage
    struct Preset {
        juce::String name;
        float gain;
        float attack;
        float decay;
        float sustain;
        float release;
        float pitchSemitones; // -12..+12
        float glideTime;      // seconds 0..0.3
        float drive;          // 0..1
        float color;          // -1..+1 (tilt)
        float toneCutoff;     // Hz (normalized in processor)
    };

    juce::OwnedArray<Preset> presets;

    void populatePresets();
    void loadPreset (int index);

    // Preset Tab UI
    std::unique_ptr<juce::TabbedComponent> tabs;

    // A simple component to host the preset list
    class PresetListComponent : public juce::Component,
                                public juce::ListBoxModel
    {
    public:
        PresetListComponent (Sub808AudioProcessorEditor& ownerRef) : owner (ownerRef)
        {
            addAndMakeVisible (listBox);
            listBox.setModel (this);
            listBox.setRowHeight (24);
        }

        int getNumRows() override;
        void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;
        void resized() override { listBox.setBounds (getLocalBounds().reduced (4)); }

    private:
        Sub808AudioProcessorEditor& owner;
        juce::ListBox listBox { "Presets", this };
    };

    std::unique_ptr<PresetListComponent> presetListComponent;

    void buildTabs();

    void setupSlider (juce::Slider& s);
    void configureLabel (juce::Label& l, const juce::String& text);
    void setupPresetBox();

    void layoutKnobRow (juce::Rectangle<int> rowArea,
                        std::initializer_list<std::pair<juce::Slider*, juce::Label*>> controls,
                        int padding = 10);

    void setupLabel (juce::Label& l, const juce::String& text)
    {
        // Typography
        l.setText (text, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);

        // Choose a modern, readable font with slight weight for dark UI
        l.setFont (juce::Font (juce::Font::getDefaultSansSerifFontName(), 13.5f, juce::Font::plain));

        // Colors tuned to match Sub808LookAndFeel palette
        auto fg = juce::Colour::fromRGB (210, 225, 240);     // primary label text
        auto glow = juce::Colour::fromRGB (120, 200, 255);   // accent glow (matches arcValue)
        auto subtle = juce::Colour::fromRGB (70, 80, 95);    // subtle outline

        l.setColour (juce::Label::textColourId, fg.withAlpha (0.95f));

        // Remove default background to blend with panel
        l.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);

        // Add subtle outline for readability over busy backgrounds
        l.setColour (juce::Label::outlineColourId, subtle.withAlpha (0.35f));

        // Disable editing and focus to keep it display-only
        l.setEditable (false, false, false);
        l.setInterceptsMouseClicks (false, false);

        // Apply a soft glow/shadow behind text for depth on dark backgrounds
        l.setBorderSize ({ 2, 6, 2, 6 });

        // Ensure we only add one DropShadower instance
        if (! static_cast<bool> (l.getProperties().getWithDefault ("hasGlow", false)))
        {
            auto shadowColour = glow.withAlpha (0.22f);
            juce::DropShadow dropShadow (shadowColour, 8, { 0, 0 });

            auto* effect = new juce::DropShadowEffect();
            effect->setShadowProperties (dropShadow);
            l.setComponentEffect (effect); // Component takes ownership

            l.getProperties().set ("hasGlow", true);
        }
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sub808AudioProcessorEditor)


};

