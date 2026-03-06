#pragma once

#include <vector>

// GM Drum Map note numbers (24 essential slots)
namespace GM {
    constexpr int Kick         = 36;  // C2
    constexpr int SideStick    = 37;  // C#2
    constexpr int Snare        = 38;  // D2
    constexpr int Clap         = 39;  // D#2
    constexpr int ElecSnare    = 40;  // E2
    constexpr int LowFloorTom  = 41;  // F2
    constexpr int ClosedHH     = 42;  // F#2
    constexpr int HighFloorTom = 43;  // G2
    constexpr int PedalHH      = 44;  // G#2
    constexpr int LowTom       = 45;  // A2
    constexpr int OpenHH       = 46;  // A#2
    constexpr int LowMidTom    = 47;  // B2
    constexpr int HighMidTom   = 48;  // C3
    constexpr int Crash1       = 49;  // C#3
    constexpr int HighTom      = 50;  // D3
    constexpr int Ride1        = 51;  // D#3
    constexpr int Chinese      = 52;  // E3
    constexpr int RideBell     = 53;  // F3
    constexpr int Tambourine   = 54;  // F#3
    constexpr int Splash       = 55;  // G3
    constexpr int Cowbell       = 56;  // G#3
    constexpr int Crash2       = 57;  // A3
    constexpr int Ride2        = 59;  // B3
    constexpr int HiBongo      = 60;  // C4
}

inline const char* midiNoteName(int note) {
    switch (note) {
        case GM::Kick:         return "Kick";
        case GM::SideStick:    return "Side Stick";
        case GM::Snare:        return "Snare";
        case GM::Clap:         return "Clap";
        case GM::ElecSnare:    return "Elec Snare";
        case GM::LowFloorTom:  return "Low Floor Tom";
        case GM::ClosedHH:     return "Closed HH";
        case GM::HighFloorTom: return "High Floor Tom";
        case GM::PedalHH:      return "Pedal HH";
        case GM::LowTom:       return "Low Tom";
        case GM::OpenHH:       return "Open HH";
        case GM::LowMidTom:    return "Low-Mid Tom";
        case GM::HighMidTom:   return "High-Mid Tom";
        case GM::Crash1:       return "Crash 1";
        case GM::HighTom:      return "High Tom";
        case GM::Ride1:        return "Ride 1";
        case GM::Chinese:      return "Chinese";
        case GM::RideBell:     return "Ride Bell";
        case GM::Tambourine:   return "Tambourine";
        case GM::Splash:       return "Splash";
        case GM::Cowbell:      return "Cowbell";
        case GM::Crash2:       return "Crash 2";
        case GM::Ride2:        return "Ride 2";
        case GM::HiBongo:      return "Hi Bongo";
        default:               return "Unknown";
    }
}

inline std::vector<int> getDefaultSlotNotes() {
    return {
        GM::Kick, GM::SideStick, GM::Snare, GM::Clap,
        GM::ElecSnare, GM::LowFloorTom, GM::ClosedHH, GM::HighFloorTom,
        GM::PedalHH, GM::LowTom, GM::OpenHH, GM::LowMidTom,
        GM::HighMidTom, GM::Crash1, GM::HighTom, GM::Ride1,
        GM::Chinese, GM::RideBell, GM::Tambourine, GM::Splash,
        GM::Cowbell, GM::Crash2, GM::Ride2, GM::HiBongo
    };
}
