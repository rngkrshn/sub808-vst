/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Sub808AudioProcessor::Sub808AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Sub808AudioProcessor::~Sub808AudioProcessor()
{
}

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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Sub808AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Sub808AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Sub808AudioProcessor::getProgramName (int index)
{
    return {};
}

void Sub808AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Sub808AudioProcessor::prepareToPlay (double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRateHz = newSampleRate;
    phase = 0.0f;
    phaseDelta = 0.0f;

    adsr.setSampleRate (sampleRateHz);

    // Phase 3 defaults (fast pluck so you hear it immediately)
    adsrParams.attack  = 0.005f;
    adsrParams.decay   = 0.05f;
    adsrParams.sustain = 0.8f;
    adsrParams.release = 0.15f;
    adsr.setParameters (adsrParams);
}

void Sub808AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Sub808AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Sub808AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

        const auto numSamples  = buffer.getNumSamples();
        const auto numChannels = buffer.getNumChannels();

        buffer.clear();

        // --- Handle MIDI (single monophonic voice) ---
        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();

            if (msg.isNoteOn())
            {
                const auto freq = (float) juce::MidiMessage::getMidiNoteInHertz (msg.getNoteNumber());
                phaseDelta = juce::MathConstants<float>::twoPi * freq / (float) sampleRateHz;

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

        // --- Render audio ---
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float env = adsr.getNextSample();

            // -12 dB-ish so it’s not blasting
            const float s = std::sin (phase) * env * 0.25f;

            phase += phaseDelta;
            if (phase >= juce::MathConstants<float>::twoPi)
                phase -= juce::MathConstants<float>::twoPi;

            for (int ch = 0; ch < numChannels; ++ch)
                buffer.setSample (ch, sample, s);    }
}

//==============================================================================
bool Sub808AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Sub808AudioProcessor::createEditor()
{
    return new Sub808AudioProcessorEditor (*this);
}

//==============================================================================
void Sub808AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Sub808AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Sub808AudioProcessor();
}
