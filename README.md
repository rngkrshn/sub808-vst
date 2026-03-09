# Sub808

**Sub808** is a minimal JUCE-based VST3/AU synthesizer focused on clean 808-style sub-bass generation.

The plugin is intentionally simple: a monophonic sine oscillator driven by MIDI and shaped with an ADSR envelope and gain control. It serves as both a functional sub-bass tool and a learning-focused audio DSP project.

---

## Features
- Monophonic sine-wave oscillator
- MIDI note input (pitch from note number)
- ADSR control (Attack, Decay, Sustain, Release)
- Gain control
- APVTS-based parameter management
- VST3 support (AU via JUCE)

---

## Parameters
- **Gain** – Output level  
- **Attack** – Envelope attack time  
- **Decay** – Envelope decay time  
- **Sustain** – Envelope sustain level  
- **Release** – Envelope release time  

---

## Built With
- C++
- JUCE
- Xcode (macOS)
- VST3 plugin format

---

## Build (macOS)
1. Clone the repo  
2. Open the JUCE-generated Xcode project  
3. Build the **VST3** target  
4. Scan the plugin in your DAW  

---

## Status
- Monophonic only  
- UI is functional, not production-polished  

---

## Roadmap
- Pitch envelope (808 drop)
- Drive/saturation
- Presets
- UI refinements
## Running the Sub808 Plugin

Sub808 is a JUCE-based audio plugin designed to run inside a Digital Audio Workstation (DAW). Follow the steps below to build and use the plugin.

### Requirements
- macOS
- Xcode
- JUCE framework
- A DAW that supports VST3 or AU plugins (such as Logic Pro, Ableton Live, FL Studio, or Reaper)

### Build Instructions

1. Clone the repository

git clone https://github.com/rngkrshn/sub808-vst.git

2. Open the project in Xcode

Navigate to the project folder and open the `.xcodeproj` file.

3. Select the build target

Choose either:
- VST3
- AU

depending on which plugin format you want to build.

4. Build the plugin

Press the **Run** or **Build** button in Xcode.  
Xcode will compile the plugin and place the build output in the build folder.

### Installing the Plugin

After building, move the compiled plugin to the appropriate plugin folder.

For AU plugins:

/Library/Audio/Plug-Ins/Components/

For VST3 plugins:

/Library/Audio/Plug-Ins/VST3/

### Using the Plugin

1. Open your DAW.
2. Refresh or rescan plugins if necessary.
3. Search for **Sub808** in the plugin list.
4. Insert the plugin on an instrument track.

The Sub808 interface will appear and can be used to generate and shape sub-bass frequencies.
