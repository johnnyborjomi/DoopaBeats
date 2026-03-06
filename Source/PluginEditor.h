#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class DoopaBeatsEditor : public juce::AudioProcessorEditor,
                         private juce::Timer {
public:
    explicit DoopaBeatsEditor(DoopaBeatsProcessor&);
    ~DoopaBeatsEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    DoopaBeatsProcessor& processor;

    // Controls
    juce::TextButton tapButton     { "TAP" };
    juce::TextButton transButton   { "TRANSITION" };
    juce::TextButton stopButton    { "STOP" };
    juce::Slider     tempoSlider;
    juce::Label      tempoLabel;
    juce::ComboBox   songSelector;

    // Display state
    int lastBeat = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoopaBeatsEditor)
};
