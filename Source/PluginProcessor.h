#pragma once

#include <JuceHeader.h>
#include "DrumEngine.h"
#include "SongPlayer.h"
#include "PatternData.h"

class DoopaBeatsProcessor : public juce::AudioProcessor {
public:
    DoopaBeatsProcessor();
    ~DoopaBeatsProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DoopaBeats"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // Public access for the editor
    SongPlayer& getSongPlayer() { return songPlayer; }
    std::vector<Song>& getSongs() { return songs; }
    void loadSong(int index);

private:
    DrumEngine drumEngine;
    SongPlayer songPlayer;
    std::vector<Song> songs;
    int currentSongIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoopaBeatsProcessor)
};
