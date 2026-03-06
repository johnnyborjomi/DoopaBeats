#pragma once

#include <JuceHeader.h>
#include "MidiNoteMap.h"
#include <map>

struct DrumSlot {
    int midiNote = 0;
    juce::String name;
    juce::File sourceFile;
    juce::AudioBuffer<float> sample;
    float pan = 0.0f;
    float gain = 1.0f;
    bool loaded = false;
};

class DrumKit {
public:
    DrumKit();

    void setName(const juce::String& name) { kitName = name; }
    juce::String getName() const { return kitName; }

    DrumSlot* getSlot(int midiNote);
    const DrumSlot* getSlot(int midiNote) const;
    const std::map<int, DrumSlot>& getSlots() const { return slots; }

    bool loadSample(int midiNote, const juce::File& wavFile, double targetSampleRate);
    void clearSlot(int midiNote);

    bool saveToFile(const juce::File& file) const;
    bool loadFromFile(const juce::File& file, double targetSampleRate);

    static juce::File getKitsDirectory();
    static juce::File getSamplesDirectory();
    static juce::Array<juce::File> findAllKitFiles();

private:
    juce::String kitName { "Untitled Kit" };
    std::map<int, DrumSlot> slots;

    static juce::AudioBuffer<float> readAndResample(const juce::File& file,
                                                     double targetSampleRate);
};
