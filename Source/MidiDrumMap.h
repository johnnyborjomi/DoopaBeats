#pragma once

#include <array>
#include <string>
#include <vector>

struct MidiDrumMap {
    std::string name;
    std::array<int, 128> noteMap;  // sourceNote -> GM note (0 = discard)
};

inline MidiDrumMap createGMStandardMap() {
    MidiDrumMap map;
    map.name = "GM Standard";
    for (int i = 0; i < 128; i++)
        map.noteMap[i] = i;
    return map;
}

inline MidiDrumMap createEZdrummerMap() {
    MidiDrumMap map;
    map.name = "EZdrummer / Toontrack";
    map.noteMap.fill(0);

    // Core GM range passes through
    for (int i = 35; i <= 59; i++)
        map.noteMap[i] = i;

    // Extended articulations below note 35
    map.noteMap[22] = 42;  // HH Closed Edge -> Closed HH
    map.noteMap[23] = 42;  // HH Closed Tip -> Closed HH
    map.noteMap[24] = 46;  // HH Open 1 -> Open HH
    map.noteMap[25] = 46;  // HH Open 2 -> Open HH
    map.noteMap[26] = 46;  // HH Open 3 -> Open HH
    map.noteMap[27] = 46;  // HH Open Edge -> Open HH
    map.noteMap[28] = 44;  // HH Pedal Chick -> Pedal HH
    map.noteMap[29] = 51;  // Ride Tip -> Ride
    map.noteMap[30] = 51;  // Ride Edge -> Ride
    map.noteMap[31] = 53;  // Ride Bell -> Ride Bell
    map.noteMap[32] = 49;  // Crash Edge -> Crash 1
    map.noteMap[33] = 49;  // Crash Muted -> Crash 1
    map.noteMap[34] = 56;  // Cowbell/Sticks -> Cowbell

    // Overrides within GM range
    map.noteMap[35] = 36;  // Acoustic BD -> Kick
    map.noteMap[40] = 38;  // Electric Snare -> Snare

    return map;
}

inline MidiDrumMap createSuperiorDrummerMap() {
    // SD3 shares the same GM Extended foundation as EZdrummer
    MidiDrumMap map = createEZdrummerMap();
    map.name = "Superior Drummer 3";

    // Additional SD3 articulations
    map.noteMap[21] = 38;  // Snare Buzz -> Snare
    map.noteMap[58] = 0;   // Vibraslap/Perc -> discard

    return map;
}

inline MidiDrumMap createAddictiveDrumsMap() {
    MidiDrumMap map;
    map.name = "Addictive Drums 2";
    map.noteMap.fill(0);

    // Core mappings
    map.noteMap[36] = 36;  // Kick
    map.noteMap[38] = 38;  // Snare Hit
    map.noteMap[37] = 37;  // Snare Sidestick
    map.noteMap[40] = 38;  // Snare Rimshot -> Snare
    map.noteMap[42] = 42;  // HH Closed
    map.noteMap[46] = 46;  // HH Open
    map.noteMap[44] = 44;  // HH Pedal
    map.noteMap[22] = 42;  // HH Seq Hit -> Closed HH
    map.noteMap[41] = 41;  // Floor Tom
    map.noteMap[43] = 43;  // Floor Tom 2
    map.noteMap[45] = 45;  // Rack Tom Low
    map.noteMap[47] = 47;  // Rack Tom Mid
    map.noteMap[48] = 48;  // Rack Tom High
    map.noteMap[50] = 50;  // Rack Tom Highest
    map.noteMap[49] = 49;  // Crash 1
    map.noteMap[57] = 57;  // Crash 2
    map.noteMap[55] = 55;  // Splash
    map.noteMap[52] = 52;  // China
    map.noteMap[51] = 51;  // Ride
    map.noteMap[53] = 53;  // Ride Bell
    map.noteMap[59] = 59;  // Ride 2
    map.noteMap[39] = 39;  // Clap/Handclap

    return map;
}

inline std::vector<MidiDrumMap> getPresetMaps() {
    return {
        createGMStandardMap(),
        createEZdrummerMap(),
        createSuperiorDrummerMap(),
        createAddictiveDrumsMap()
    };
}

inline std::vector<std::string> getPresetNames() {
    std::vector<std::string> names;
    for (auto& m : getPresetMaps())
        names.push_back(m.name);
    return names;
}
