#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DrumKitEditor.h"

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
    juce::TextButton kitButton     { "KIT" };
    juce::Slider     tempoSlider;
    juce::Label      tempoLabel;
    juce::ComboBox   songSelector;

    // Kit editor overlay
    DrumKitEditor kitEditor;
    bool showingKitView = false;

    // Display state
    int lastBeat = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoopaBeatsEditor)
};
