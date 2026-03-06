#pragma once

#include <JuceHeader.h>
#include "PatternData.h"
#include "MidiDrumMap.h"

struct ImportSettings {
    int stepsPerBeat = 4;   // 4 = 16th notes, 3 = triplets
    int beatsPerBar = 4;
    int maxBars = 8;
    const MidiDrumMap* drumMap = nullptr;  // null = no remapping (GM passthrough)
};

class MidiImporter {
public:
    static Pattern importFromFile(const juce::File& file, const ImportSettings& settings = {});
};
