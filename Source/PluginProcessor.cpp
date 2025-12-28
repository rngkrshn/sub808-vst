/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ============================================================================== 
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Parameter layout (must match your editor attachment IDs)
juce::AudioProcessorValueTreeState::ParameterLayout
Sub808AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "gain", "Gain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.25f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "attack", "Attack",
        juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f, 0.4f),
        0.005f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "decay", "Decay",
        juce::NormalisableRange<float> (0.001f, 2.0f, 0.001f, 0.4f),
        0.05f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "sustain", "Sustain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.8f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "release", "Release",
        juce::NormalisableRange<float> (0.001f, 5.0f, 0.001f, 0.4f),
        0.15f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "pitchSemitones", "Pitch",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.01f),
        0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "glideTime", "Glide",
        juce::NormalisableRange<float> (0.0f, 0.3f, 0.0001f, 0.4f),
        0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.0001f),
        0.1f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "color", "Color",
        juce::NormalisableRange<float> (-1.0f, 1.0f, 0.0001f),
        0.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat>(
        "toneCutoff", "Tone Cutoff",
        juce::NormalisableRange<float> (80.0f, 8000.0f, 0.01f, 0.25f),
        300.0f));

    return { params.begin(), params.end() };
}

//==============================================================================

Sub808AudioProcessor::Sub808AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
#else
    : AudioProcessor()
#endif
    , apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

Sub808AudioProcessor::~Sub808AudioProcessor() = default;

//==============================================================================

const juce::String Sub808AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Sub808AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Sub808AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Sub808AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Sub808AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Sub808AudioProcessor::getNumPrograms()
{
    return 1;
}

int Sub808AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sub808AudioProcessor::setCurrentProgram (int /*index*/)
{
}

const juce::String Sub808AudioProcessor::getProgramName (int /*index*/)
{
    return {};
}

void Sub808AudioProcessor::changeProgramName (int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================

void Sub808AudioProcessor::prepareToPlay (double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRateHz = newSampleRate;
    phase = 0.0f;
    phaseDelta = 0.0f;

    adsr.setSampleRate (sampleRateHz);

    adsrParams.attack  = 0.005f;
    adsrParams.decay   = 0.05f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.15f;
    adsr.setParameters (adsrParams);

    currentFreq = 0.0f;
    targetFreq  = 0.0f;
    glideSamplesRemaining = 0;

    for (int ch = 0; ch < 2; ++ch)
        toneZ[ch] = 0.0f;
}

void Sub808AudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sub808AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Support mono or stereo output only
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono()
     && mainOut != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    // If not a synth, require matching input/output layouts
    if (mainOut != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Sub808AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const float gain    = apvts.getRawParameterValue ("gain")->load();
    const float attack  = apvts.getRawParameterValue ("attack")->load();
    const float decay   = apvts.getRawParameterValue ("decay")->load();
    const float sustain = apvts.getRawParameterValue ("sustain")->load();
    const float release = apvts.getRawParameterValue ("release")->load();

    const float pitchSemis = apvts.getRawParameterValue ("pitchSemitones")->load();
    const float glideSec   = apvts.getRawParameterValue ("glideTime")->load();
    const float driveAmt   = apvts.getRawParameterValue ("drive")->load();
    const float colorAmt   = apvts.getRawParameterValue ("color")->load();
    const float toneHz     = apvts.getRawParameterValue ("toneCutoff")->load();

    juce::ADSR::Parameters p;
    p.attack  = attack;
    p.decay   = decay;
    p.sustain = sustain;
    p.release = release;
    adsr.setParameters (p);

    const float toneAlpha = juce::jlimit (0.0f, 1.0f, (float) std::exp (-2.0f * juce::MathConstants<float>::pi * toneHz / (float) sampleRateHz));

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            const float base = (float) juce::MidiMessage::getMidiNoteInHertz (msg.getNoteNumber());
            const float detune = std::pow (2.0f, pitchSemis / 12.0f);
            const float newTarget = base * detune;

            targetFreq = newTarget;

            if (glideSec > 0.0f && currentFreq > 0.0f)
            {
                glideSamplesRemaining = (int) (juce::jlimit (0.0f, 10.0f, glideSec) * (float) sampleRateHz);
                if (glideSamplesRemaining <= 0)
                {
                    currentFreq = targetFreq;
                    phaseDelta = juce::MathConstants<float>::twoPi * currentFreq / (float) sampleRateHz;
                }
            }
            else
            {
                currentFreq = targetFreq;
                phaseDelta = juce::MathConstants<float>::twoPi * currentFreq / (float) sampleRateHz;
            }

            adsr.noteOn();
        }
        else if (msg.isNoteOff())
        {
            adsr.noteOff();
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            adsr.reset();
        }
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Glide toward target frequency
        if (glideSamplesRemaining > 0)
        {
            const float step = (targetFreq - currentFreq) / (float) glideSamplesRemaining;
            currentFreq += step;
            --glideSamplesRemaining;
            phaseDelta = juce::MathConstants<float>::twoPi * currentFreq / (float) sampleRateHz;
        }

        const float env = adsr.getNextSample();
        float s = std::sin (phase) * env;

        // Soft saturation (drive): arctangent waveshaper
        if (driveAmt > 0.0f)
        {
            const float k = juce::jmap (driveAmt, 0.0f, 1.0f, 0.0f, 2.5f);
            s = std::tanh (k * s) / (k > 0.0f ? std::tanh (k) : 1.0f);
        }

        // Simple tilt EQ (color): brighten (positive) or warm (negative)
        // Apply as pre-emphasis/de-emphasis using a simple high-shelf approximation
        float low = s;
        float high = s - toneZ[0]; // crude high-passed component based on previous low (mono ref)
        s = s + high * colorAmt * 0.5f - low * (-colorAmt) * 0.5f;

        // One-pole low-pass tone filter per channel later when writing

        phase += phaseDelta;
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            // Apply tone LPF per channel
            float z = toneZ[ch];
            z = toneAlpha * z + (1.0f - toneAlpha) * s;
            toneZ[ch] = z;

            const float out = z * gain;
            buffer.setSample (ch, sample, out);
        }
    }
}

//==============================================================================

bool Sub808AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* Sub808AudioProcessor::createEditor()
{
    return new Sub808AudioProcessorEditor (*this);
}

//==============================================================================

void Sub808AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Sub808AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sub808AudioProcessor();
}

