/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_audio_processors/juce_audio_processors.h>
//==============================================================================
/**
*/
class Sub808AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Sub808AudioProcessor();
    ~Sub808AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    //==============================================================================
    
    double sampleRateHz = 44100.0;
    float phase = 0.0f;
    float phaseDelta = 0.0f;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    // New DSP state for pitch/glide/tone
    float currentFreq = 0.0f;
    float targetFreq  = 0.0f;
    int   glideSamplesRemaining = 0;
    float toneZ[2] { 0.0f, 0.0f };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sub808AudioProcessor)
};

