#include "PluginProcessor.h"
#include "PluginEditor.h"

Sub808AudioProcessorEditor::Sub808AudioProcessorEditor (Sub808AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&lnf);

    setupSlider (gainSlider);
    setupSlider (attackSlider);
    setupSlider (decaySlider);
    setupSlider (sustainSlider);
    setupSlider (releaseSlider);

    setupSlider (pitchSlider);
    setupSlider (glideSlider);
    setupSlider (driveSlider);
    setupSlider (colorSlider);
    setupSlider (toneSlider);

    gainAttach    = std::make_unique<Attachment> (audioProcessor.apvts, "gain",           gainSlider);
    attackAttach  = std::make_unique<Attachment> (audioProcessor.apvts, "attack",         attackSlider);
    decayAttach   = std::make_unique<Attachment> (audioProcessor.apvts, "decay",          decaySlider);
    sustainAttach = std::make_unique<Attachment> (audioProcessor.apvts, "sustain",        sustainSlider);
    releaseAttach = std::make_unique<Attachment> (audioProcessor.apvts, "release",        releaseSlider);

    pitchAttach  = std::make_unique<Attachment> (audioProcessor.apvts, "pitchSemitones", pitchSlider);
    glideAttach  = std::make_unique<Attachment> (audioProcessor.apvts, "glideTime",      glideSlider);
    driveAttach  = std::make_unique<Attachment> (audioProcessor.apvts, "drive",          driveSlider);
    colorAttach  = std::make_unique<Attachment> (audioProcessor.apvts, "color",          colorSlider);
    toneAttach   = std::make_unique<Attachment> (audioProcessor.apvts, "toneCutoff",     toneSlider);

    addAndMakeVisible (gainSlider);
    addAndMakeVisible (attackSlider);
    addAndMakeVisible (decaySlider);
    addAndMakeVisible (sustainSlider);
    addAndMakeVisible (releaseSlider);

    addAndMakeVisible (pitchSlider);
    addAndMakeVisible (glideSlider);
    addAndMakeVisible (driveSlider);
    addAndMakeVisible (colorSlider);
    addAndMakeVisible (toneSlider);

    configureLabel (gainLabel,    "GAIN");
    configureLabel (attackLabel,  "ATTACK");
    configureLabel (decayLabel,   "DECAY");
    configureLabel (sustainLabel, "SUSTAIN");
    configureLabel (releaseLabel, "RELEASE");

    configureLabel (pitchLabel, "PITCH");
    configureLabel (glideLabel, "GLIDE");
    configureLabel (driveLabel, "DRIVE");
    configureLabel (colorLabel, "COLOR");
    configureLabel (toneLabel,  "TONE");

    setupPresetBox();
    populatePresets();

    // Removed tabs creation and buildTabs() since tabs are not used now
    // tabs = std::make_unique<juce::TabbedComponent>(juce::TabbedButtonBar::TabsAtTop);
    // addAndMakeVisible (*tabs);
    // buildTabs();

    setSize (640, 260);
}

Sub808AudioProcessorEditor::~Sub808AudioProcessorEditor()
{
    presetBox.onChange = nullptr;
    setLookAndFeel (nullptr);
}

void Sub808AudioProcessorEditor::buildTabs()
{
    presetListComponent = std::make_unique<PresetListComponent> (*this);

    auto* presetContainer = new juce::Component();
    presetContainer->addAndMakeVisible (*presetListComponent);

    // We'll set the bounds of presetListComponent in resized(), so no onResize lambda here.

    tabs->clearTabs();
    tabs->addTab ("Presets", juce::Colours::transparentBlack, presetContainer, true);

    if (auto* content = tabs->getTabContentComponent (0))
        presetListComponent->setBounds (content->getLocalBounds());
}

int Sub808AudioProcessorEditor::PresetListComponent::getNumRows()
{
    return owner.presets.size();
}

void Sub808AudioProcessorEditor::PresetListComponent::paintListBoxItem (int row, juce::Graphics& g,
                                                                        int width, int height, bool rowIsSelected)
{
    auto bg = rowIsSelected ? juce::Colour::fromRGB (50, 60, 75)
                            : juce::Colour::fromRGB (28, 28, 34);
    g.fillAll (bg);

    if (juce::isPositiveAndBelow (row, owner.presets.size()))
    {
        auto* p = owner.presets[row];
        g.setColour (juce::Colour::fromRGB (210, 225, 240));
        g.drawText (p->name, 8, 0, width - 16, height, juce::Justification::centredLeft, true);
    }
}

void Sub808AudioProcessorEditor::PresetListComponent::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow (row, owner.presets.size()))
    {
        owner.loadPreset (row);
        owner.presetBox.setSelectedId (row + 1, juce::dontSendNotification);
    }
}

void Sub808AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 18, 22));

    // Top bar
    auto topBar = getLocalBounds().removeFromTop (44);
    g.setColour (juce::Colour::fromRGB (28, 28, 36));
    g.fillRoundedRectangle (topBar.toFloat(), 10.0f);

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (20.0f, juce::Font::bold));
    g.drawText ("Sub808", topBar.reduced (12, 8), juce::Justification::centredLeft);
}

void Sub808AudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Top bar (44 px)
    auto topBar = area.removeFromTop (44);
    {
        const int comboW = 220;
        const int comboH = 26;
        auto right = topBar.reduced (12, 9);
        presetBox.setBounds ({ right.getRight() - comboW, right.getY(), comboW, comboH });
    }

    // Controls area: two rows with padding
    auto controlsArea = area.reduced (8, 6);
    auto rowHeight = (controlsArea.getHeight() - 16) / 2;
    auto row1 = controlsArea.removeFromTop (rowHeight);
    controlsArea.removeFromTop (16);
    auto row2 = controlsArea;

    layoutKnobRow (row1, {
        { &attackSlider,  &attackLabel },
        { &decaySlider,   &decayLabel },
        { &sustainSlider, &sustainLabel },
        { &releaseSlider, &releaseLabel },
        { &gainSlider,    &gainLabel }
    });

    layoutKnobRow (row2, {
        { &pitchSlider, &pitchLabel },
        { &glideSlider, &glideLabel },
        { &driveSlider, &driveLabel },
        { &colorSlider, &colorLabel },
        { &toneSlider,  &toneLabel }
    });

    // Removed setting presetListComponent bounds since tabs are no longer used
}

void Sub808AudioProcessorEditor::layoutKnobRow (juce::Rectangle<int> rowArea,
                                                std::initializer_list<std::pair<juce::Slider*, juce::Label*>> controls,
                                                int padding)
{
    const int count = (int) controls.size();
    if (count == 0) return;
    const int cellW = rowArea.getWidth() / count;
    const int labelH = 18;
    const int topPad = 6;

    int i = 0;
    for (auto pair : controls)
    {
        auto cell = rowArea.withX (rowArea.getX() + i * cellW).withWidth (cellW).reduced (padding);
        auto labelArea = juce::Rectangle<int> (cell.getX(), cell.getY(), cell.getWidth(), labelH);
        auto knobArea  = cell.withTrimmedTop (labelH + topPad);
        if (pair.second) pair.second->setBounds (labelArea);
        if (pair.first)  pair.first ->setBounds (knobArea);
        ++i;
    }
}

void Sub808AudioProcessorEditor::setupSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 22);

    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour::fromRGB (28, 28, 36));
}

void Sub808AudioProcessorEditor::configureLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, juce::Colour::fromRGB (200, 200, 210));
    l.setFont (juce::Font (12.0f, juce::Font::bold));
    addAndMakeVisible (l);
}

void Sub808AudioProcessorEditor::setupPresetBox()
{
    addAndMakeVisible (presetBox);
    presetBox.setTooltip ("Presets");
    presetBox.setJustificationType (juce::Justification::centred);
    presetBox.onChange = [this]
    {
        auto idx = presetBox.getSelectedId() - 1;
        if (idx >= 0)
            loadPreset (idx);
    };
}

void Sub808AudioProcessorEditor::populatePresets()
{
    presets.clear();
    presetBox.clear();

    // Preset { name, gain, attack, decay, sustain, release, pitchSemitones, glideTime, drive, color, toneCutoff }
    presets.add (new Preset { "Default Clean 808", 0.70f, 0.02f, 0.20f, 0.60f, 0.30f, 0.0f, 0.00f, 0.10f,  0.00f, 300.0f });
    presets.add (new Preset { "Spinz 808",          0.85f, 0.00f, 0.12f, 0.55f, 0.20f, 0.0f, 0.02f, 0.45f,  0.35f, 450.0f });
    presets.add (new Preset { "Zay 808",            0.80f, 0.01f, 0.25f, 0.65f, 0.30f, 0.0f, 0.03f, 0.25f, -0.20f, 280.0f });
    presets.add (new Preset { "Subby Glide",        0.75f, 0.02f, 0.30f, 0.60f, 0.40f, -2.0f, 0.12f, 0.15f, -0.10f, 220.0f });
    presets.add (new Preset { "Punch 808",          0.90f, 0.00f, 0.10f, 0.50f, 0.18f, 0.0f, 0.01f, 0.50f,  0.40f, 520.0f });
    presets.add (new Preset { "Warm Tape 808",      0.78f, 0.02f, 0.22f, 0.62f, 0.28f, -1.0f, 0.02f, 0.30f, -0.35f, 260.0f });
    presets.add (new Preset { "Distorted 808",      0.95f, 0.00f, 0.12f, 0.50f, 0.22f, 0.0f, 0.00f, 0.75f,  0.45f, 600.0f });
    presets.add (new Preset { "Long Boom",          0.80f, 0.01f, 0.40f, 0.60f, 0.60f, 0.0f, 0.00f, 0.20f, -0.25f, 240.0f });
    presets.add (new Preset { "Soft Attack 808",    0.70f, 0.06f, 0.28f, 0.58f, 0.35f, 0.0f, 0.00f, 0.15f, -0.15f, 300.0f });
    presets.add (new Preset { "Tight Click 808",    0.88f, 0.00f, 0.08f, 0.45f, 0.15f, 0.0f, 0.00f, 0.40f,  0.50f, 650.0f });

    for (int i = 0; i < presets.size(); ++i)
        presetBox.addItem (presets[i]->name, i + 1);

    presetBox.setSelectedId (1, juce::dontSendNotification);
    loadPreset (0);
}

void Sub808AudioProcessorEditor::loadPreset (int index)
{
    if (index < 0 || index >= presets.size())
        return;

    if (! audioProcessor.apvts.getParameter("gain") ||
        ! audioProcessor.apvts.getParameter("attack") ||
        ! audioProcessor.apvts.getParameter("decay") ||
        ! audioProcessor.apvts.getParameter("sustain") ||
        ! audioProcessor.apvts.getParameter("release") ||
        ! audioProcessor.apvts.getParameter("pitchSemitones") ||
        ! audioProcessor.apvts.getParameter("glideTime") ||
        ! audioProcessor.apvts.getParameter("drive") ||
        ! audioProcessor.apvts.getParameter("color") ||
        ! audioProcessor.apvts.getParameter("toneCutoff"))
        return;

    auto& preset = *presets.getUnchecked(index);

    auto* gainParam    = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("gain"));
    auto* attackParam  = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("attack"));
    auto* decayParam   = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("decay"));
    auto* sustainParam = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("sustain"));
    auto* releaseParam = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("release"));
    auto* pitchParam   = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("pitchSemitones"));
    auto* glideParam   = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("glideTime"));
    auto* driveParam   = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("drive"));
    auto* colorParam   = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("color"));
    auto* toneParam    = dynamic_cast<juce::RangedAudioParameter*> (audioProcessor.apvts.getParameter ("toneCutoff"));

    if (gainParam)    gainParam->setValueNotifyingHost (gainParam->convertTo0to1 (preset.gain));
    if (attackParam)  attackParam->setValueNotifyingHost (attackParam->convertTo0to1 (preset.attack));
    if (decayParam)   decayParam->setValueNotifyingHost (decayParam->convertTo0to1 (preset.decay));
    if (sustainParam) sustainParam->setValueNotifyingHost (sustainParam->convertTo0to1 (preset.sustain));
    if (releaseParam) releaseParam->setValueNotifyingHost (releaseParam->convertTo0to1 (preset.release));
    if (pitchParam)   pitchParam->setValueNotifyingHost (pitchParam->convertTo0to1 (preset.pitchSemitones));
    if (glideParam)   glideParam->setValueNotifyingHost (glideParam->convertTo0to1 (preset.glideTime));
    if (driveParam)   driveParam->setValueNotifyingHost (driveParam->convertTo0to1 (preset.drive));
    if (colorParam)   colorParam->setValueNotifyingHost (colorParam->convertTo0to1 (preset.color));
    if (toneParam)    toneParam->setValueNotifyingHost (toneParam->convertTo0to1 (preset.toneCutoff));

    // Update sliders immediately to reflect preset change
    gainSlider.setValue(preset.gain, juce::dontSendNotification);
    attackSlider.setValue(preset.attack, juce::dontSendNotification);
    decaySlider.setValue(preset.decay, juce::dontSendNotification);
    sustainSlider.setValue(preset.sustain, juce::dontSendNotification);
    releaseSlider.setValue(preset.release, juce::dontSendNotification);

    pitchSlider.setValue(preset.pitchSemitones, juce::dontSendNotification);
    glideSlider.setValue(preset.glideTime, juce::dontSendNotification);
    driveSlider.setValue(preset.drive, juce::dontSendNotification);
    colorSlider.setValue(preset.color, juce::dontSendNotification);
    toneSlider.setValue(preset.toneCutoff, juce::dontSendNotification);
}

