#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DrumKitEditor.h"
#include "SongEditor.h"

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
    juce::TextButton songButton    { "SONG" };
    juce::Slider     tempoSlider;
    juce::Label      tempoLabel;
    juce::ComboBox   songSelector;

    // Editor overlays
    DrumKitEditor kitEditor;
    SongEditor songEditor;
    bool showingKitView = false;
    bool showingSongView = false;

    // Display state
    int lastBeat = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoopaBeatsEditor)
};
