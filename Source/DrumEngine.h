#pragma once

#include <JuceHeader.h>
#include "PatternData.h"
#include <array>

class DrumEngine {
public:
    void prepare(double sampleRate, int blockSize);
    void trigger(DrumType drum, float velocity, int sampleOffset);
    void renderBlock(juce::AudioBuffer<float>& buffer);

private:
    struct Voice {
        juce::AudioBuffer<float> sample;  // pre-rendered mono sound
        int position = 0;
        int blockStartOffset = 0;
        float velocity = 0.0f;
        float panL = 1.0f;
        float panR = 1.0f;
        bool playing = false;
        bool justTriggered = false;
    };

    std::array<Voice, (int)DrumType::NumTypes> voices;
    double currentSampleRate = 44100.0;
    juce::Random rng;

    void synthesizeAllSounds();
    void synthesizeKick(juce::AudioBuffer<float>& buf);
    void synthesizeSnare(juce::AudioBuffer<float>& buf);
    void synthesizeClosedHH(juce::AudioBuffer<float>& buf);
    void synthesizeOpenHH(juce::AudioBuffer<float>& buf);
    void synthesizeClap(juce::AudioBuffer<float>& buf);
    void synthesizeLowTom(juce::AudioBuffer<float>& buf);
    void synthesizeRimshot(juce::AudioBuffer<float>& buf);
    void synthesizeRide(juce::AudioBuffer<float>& buf);

    void setPan(DrumType drum, float pan);  // -1 left, 0 center, +1 right
};
