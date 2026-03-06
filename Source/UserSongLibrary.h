#pragma once

#include <JuceHeader.h>
#include "PatternData.h"

class UserSongLibrary {
public:
    static juce::File getSongsDirectory();
    static juce::Array<juce::File> findAllSongFiles();
    static bool saveSong(const Song& song, const juce::File& file);
    static Song loadSong(const juce::File& file);
};
