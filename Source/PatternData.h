#pragma once

#include <vector>
#include <string>
#include <array>
#include <map>
#include <algorithm>
#include <initializer_list>
#include <utility>

enum class DrumType : int {
    Kick = 0,
    Snare,
    ClosedHH,
    OpenHH,
    Clap,
    LowTom,
    Rimshot,
    Ride,
    NumTypes
};

inline const char* drumTypeName(DrumType d) {
    switch (d) {
        case DrumType::Kick:     return "Kick";
        case DrumType::Snare:    return "Snare";
        case DrumType::ClosedHH: return "Closed HH";
        case DrumType::OpenHH:   return "Open HH";
        case DrumType::Clap:     return "Clap";
        case DrumType::LowTom:   return "Low Tom";
        case DrumType::Rimshot:  return "Rimshot";
        case DrumType::Ride:     return "Ride";
        default:                 return "?";
    }
}

struct Hit {
    int step;
    DrumType drum;
    float velocity;
};

struct Pattern {
    std::string name;
    int numSteps = 16;
    int stepsPerBeat = 4;   // 4 = 16th notes
    int beatsPerBar = 4;
    std::vector<Hit> hits;
};

struct Song {
    std::string name;
    float defaultTempo = 120.0f;
    Pattern intro;
    std::vector<Pattern> mainLoops;  // multiple parts (verse/chorus)
    std::vector<Pattern> fills;      // one fill per main loop
    std::vector<Pattern> transitions;
    Pattern outro;
    bool hasIntro = false;
    bool hasOutro = false;
};

// Helper: build a Pattern from readable string grid
// Characters: 'X' = loud (1.0), 'x' = medium (0.6), 'o' = soft (0.3), '.' = rest
inline Pattern makePattern(const std::string& name, int beatsPerBar, int stepsPerBeat,
                           std::initializer_list<std::pair<DrumType, std::string>> grid) {
    Pattern p;
    p.name = name;
    p.beatsPerBar = beatsPerBar;
    p.stepsPerBeat = stepsPerBeat;
    p.numSteps = 0;

    for (auto& [drum, steps] : grid) {
        p.numSteps = std::max(p.numSteps, (int)steps.size());
        for (int i = 0; i < (int)steps.size(); i++) {
            float vel = 0.0f;
            if (steps[i] == 'X') vel = 1.0f;
            else if (steps[i] == 'x') vel = 0.6f;
            else if (steps[i] == 'o') vel = 0.3f;
            if (vel > 0.0f)
                p.hits.push_back({i, drum, vel});
        }
    }
    return p;
}

// ─── Built-in songs ───────────────────────────────────────────

namespace BuiltInSongs {

inline Song basicRock() {
    Song s;
    s.name = "Basic Rock";
    s.defaultTempo = 120.0f;
    s.hasIntro = true;
    s.hasOutro = true;

    //                   1 e & a 2 e & a 3 e & a 4 e & a
    s.intro = makePattern("Count In", 4, 4, {
        {DrumType::ClosedHH, "X...X...X...x..X"},
    });

    // Main 1 — straight rock (2 bars)
    s.mainLoops.push_back(makePattern("Verse", 4, 4, {
        {DrumType::Kick,     "X.......X......." "X.......X......."},
        {DrumType::Snare,    "....X.......X..." "....X.......X..."},
        {DrumType::ClosedHH, "X.x.X.x.X.x.X.x" "X.x.X.x.X.x.X.x"},
    }));

    // Main 2 — more drive
    s.mainLoops.push_back(makePattern("Chorus", 4, 4, {
        {DrumType::Kick,     "X..X..X.X......." "X..X..X.X......."},
        {DrumType::Snare,    "....X.......X..." "....X.......X..X"},
        {DrumType::ClosedHH, "X.X.X.X.X.X.X.X" "X.X.X.X.X.X.X.X"},
    }));

    // Fills (one per main loop)
    s.fills.push_back(makePattern("Fill 1", 4, 4, {
        {DrumType::Kick,     "X...........X.X."},
        {DrumType::Snare,    "....X...X.X.X.X."},
        {DrumType::ClosedHH, "X.x............."},
    }));

    s.fills.push_back(makePattern("Fill 2", 4, 4, {
        {DrumType::Kick,     "X.......X......."},
        {DrumType::Snare,    "....X.X.X.X.XXXX"},
        {DrumType::LowTom,   "............X.X."},
    }));

    // Transition
    s.transitions.push_back(makePattern("Build", 4, 4, {
        {DrumType::Kick,     "X.......X......."},
        {DrumType::Snare,    "X.X.X.X.XXXXXXXX"},
    }));

    s.outro = makePattern("Ending", 4, 4, {
        {DrumType::Kick,     "X...........X..."},
        {DrumType::Snare,    "............X..."},
        {DrumType::Ride,     "X.......X......."},
    });

    return s;
}

inline Song popRock() {
    Song s;
    s.name = "Pop Rock";
    s.defaultTempo = 110.0f;
    s.hasIntro = true;
    s.hasOutro = true;

    s.intro = makePattern("Count In", 4, 4, {
        {DrumType::Rimshot, "X...X...X...X..."},
    });

    s.mainLoops.push_back(makePattern("Verse", 4, 4, {
        {DrumType::Kick,     "X.....X.X......." "X.....X.X......."},
        {DrumType::Snare,    "....X.......X..." "....X.......X..."},
        {DrumType::ClosedHH, "XxXxXxXxXxXxXxXx" "XxXxXxXxXxXxXxXx"},
    }));

    s.mainLoops.push_back(makePattern("Chorus", 4, 4, {
        {DrumType::Kick,     "X.X...X.X......." "X.X...X.X..X...."},
        {DrumType::Snare,    "....X.......X..." "....X.......X..."},
        {DrumType::Ride,     "X.x.X.x.X.x.X.x" "X.x.X.x.X.x.X.x"},
    }));

    s.fills.push_back(makePattern("Fill 1", 4, 4, {
        {DrumType::Snare,    "....X...X.X.X.X."},
        {DrumType::LowTom,   "X.......X......."},
    }));

    s.fills.push_back(makePattern("Fill 2", 4, 4, {
        {DrumType::Kick,     "X...X..........."},
        {DrumType::Snare,    "........X.X.XXXX"},
        {DrumType::LowTom,   "....X.X........."},
    }));

    s.transitions.push_back(makePattern("Transition", 4, 4, {
        {DrumType::Kick,     "X.......X......."},
        {DrumType::Snare,    "..X...X.X.X.XXXX"},
        {DrumType::OpenHH,   "X..............."},
    }));

    s.outro = makePattern("Ending", 4, 4, {
        {DrumType::Kick,     "X.............X."},
        {DrumType::Snare,    "..............X."},
        {DrumType::Ride,     "X..............."},
    });

    return s;
}

inline Song blues() {
    Song s;
    s.name = "Blues Shuffle";
    s.defaultTempo = 85.0f;
    s.hasIntro = true;
    s.hasOutro = true;

    // Triplet grid: 3 steps per beat, 12 steps per bar
    s.intro = makePattern("Count In", 4, 3, {
        {DrumType::ClosedHH, "X..X..X..X.."},
    });

    // Shuffle: hit on 1 and 3 of each triplet group = swing feel
    s.mainLoops.push_back(makePattern("Verse", 4, 3, {
        {DrumType::Kick,     "X........X.." "X........X.."},
        {DrumType::Snare,    "...X........" "...X........"},
        {DrumType::ClosedHH, "X.xX.xX.xX.x" "X.xX.xX.xX.x"},
    }));

    s.mainLoops.push_back(makePattern("Chorus", 4, 3, {
        {DrumType::Kick,     "X..X..X..X.." "X..X..X..X.."},
        {DrumType::Snare,    "...X.....X.." "...X.....X.."},
        {DrumType::Ride,     "X.xX.xX.xX.x" "X.xX.xX.xX.x"},
    }));

    s.fills.push_back(makePattern("Fill 1", 4, 3, {
        {DrumType::Snare,    "...X..X..XXX"},
        {DrumType::Kick,     "X..........."},
    }));

    s.fills.push_back(makePattern("Fill 2", 4, 3, {
        {DrumType::Snare,    "...X..XXXXXX"},
        {DrumType::Kick,     "X..X........"},
    }));

    s.transitions.push_back(makePattern("Transition", 4, 3, {
        {DrumType::Snare,    "X..X..X.XXXX"},
        {DrumType::Kick,     "X........X.."},
    }));

    s.outro = makePattern("Ending", 4, 3, {
        {DrumType::Kick,     "X.........X."},
        {DrumType::Snare,    "..........X."},
        {DrumType::Ride,     "X..........."},
    });

    return s;
}

inline std::vector<Song> getAllSongs() {
    return { basicRock(), popRock(), blues() };
}

} // namespace BuiltInSongs
