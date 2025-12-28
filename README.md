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
