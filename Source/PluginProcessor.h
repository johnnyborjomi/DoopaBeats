#pragma once

#include <JuceHeader.h>
#include "DrumEngine.h"
#include "SongPlayer.h"
#include "PatternData.h"
#include "DrumKit.h"
#include "UserSongLibrary.h"
#include "MidiDrumMap.h"

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

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public access for the editor
    SongPlayer& getSongPlayer() { return songPlayer; }
    DrumEngine& getDrumEngine() { return drumEngine; }
    std::vector<Song>& getSongs() { return songs; }
    void loadSong(int index);

    // Kit management
    void loadDrumKit(const juce::String& kitName);
    void loadBuiltInKit();
    void refreshAvailableKits();
    juce::StringArray getAvailableKits() const;
    juce::String getCurrentKitName() const { return currentKitName; }
    DrumKit& getCurrentKit() { return currentKit; }
    bool isUsingBuiltInKit() const { return usingBuiltIn; }

    // Drum map presets
    int getCurrentDrumMapIndex() const { return currentDrumMapIndex; }
    void setDrumMapIndex(int index);
    const MidiDrumMap& getCurrentDrumMap() const;

    // User song management
    int getCurrentSongIndex() const { return currentSongIndex; }
    int getBuiltInSongCount() const { return builtInSongCount; }
    bool isUserSong(int index) const { return index >= builtInSongCount; }
    void addUserSong(const Song& song);
    void removeUserSong(int index);
    void updateUserSong(int index, const Song& song);
    void refreshUserSongs();

private:
    DrumEngine drumEngine;
    SongPlayer songPlayer;
    std::vector<Song> songs;
    int builtInSongCount = 0;
    int currentSongIndex = 0;

    DrumKit currentKit;
    juce::String currentKitName { "Built-In" };
    bool usingBuiltIn = true;
    juce::StringArray availableKitNames;
    double lastSampleRate = 44100.0;

    std::vector<MidiDrumMap> drumMapPresets { getPresetMaps() };
    int currentDrumMapIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoopaBeatsProcessor)
};
