#pragma once

#include <JuceHeader.h>
#include "PatternData.h"
#include "MidiNoteMap.h"
#include <array>

class DrumKit;

class DrumEngine {
public:
    void prepare(double sampleRate, int blockSize);

    // MIDI-note-based trigger (primary interface)
    void trigger(int midiNote, float velocity, int sampleOffset);

    // Legacy DrumType trigger (delegates to MIDI note version)
    void trigger(DrumType drum, float velocity, int sampleOffset);

    void renderBlock(juce::AudioBuffer<float>& buffer);

    // Kit loading
    void loadKit(const DrumKit& kit);
    void loadBuiltInKit();

    juce::SpinLock& getLock() { return lock; }

private:
    struct Voice {
        juce::AudioBuffer<float> sample;  // pre-rendered mono sound
        int position = 0;
        int blockStartOffset = 0;
        float velocity = 0.0f;
        float gain = 1.0f;
        float panL = 1.0f;
        float panR = 1.0f;
        bool playing = false;
        bool justTriggered = false;
    };

    std::array<Voice, 128> voices;
    double currentSampleRate = 44100.0;
    juce::Random rng;
    juce::SpinLock lock;

    void synthesizeAllSounds();
    void synthesizeKick(juce::AudioBuffer<float>& buf);
    void synthesizeSnare(juce::AudioBuffer<float>& buf);
    void synthesizeClosedHH(juce::AudioBuffer<float>& buf);
    void synthesizeOpenHH(juce::AudioBuffer<float>& buf);
    void synthesizeClap(juce::AudioBuffer<float>& buf);
    void synthesizeLowTom(juce::AudioBuffer<float>& buf);
    void synthesizeRimshot(juce::AudioBuffer<float>& buf);
    void synthesizeRide(juce::AudioBuffer<float>& buf);

    void setPan(int midiNote, float pan);
};
