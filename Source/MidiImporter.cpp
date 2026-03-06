#include "MidiImporter.h"
#include "MidiNoteMap.h"
#include <map>

Pattern MidiImporter::importFromFile(const juce::File& file, const ImportSettings& settings) {
    Pattern pattern;
    pattern.name = file.getFileNameWithoutExtension().toStdString();
    pattern.stepsPerBeat = settings.stepsPerBeat;
    pattern.beatsPerBar = settings.beatsPerBar;

    juce::FileInputStream stream(file);
    if (!stream.openedOk()) return pattern;

    juce::MidiFile midiFile;
    if (!midiFile.readFrom(stream)) return pattern;

    midiFile.convertTimestampTicksToSeconds();

    // Extract tempo from first tempo meta-event (default 120 BPM)
    float bpm = 120.0f;
    for (int t = 0; t < midiFile.getNumTracks(); t++) {
        auto* track = midiFile.getTrack(t);
        if (!track) continue;
        for (int e = 0; e < track->getNumEvents(); e++) {
            auto& event = track->getEventPointer(e)->message;
            if (event.isTempoMetaEvent()) {
                bpm = (float)(60.0 / event.getTempoSecondsPerQuarterNote());
                break;
            }
        }
        if (bpm != 120.0f) break;
    }

    // Find best drum track: prefer channel 10, fallback to track with most notes
    int bestTrack = -1;
    int bestCount = 0;
    bool foundCh10 = false;

    for (int t = 0; t < midiFile.getNumTracks(); t++) {
        auto* track = midiFile.getTrack(t);
        if (!track) continue;

        int ch10Count = 0;
        int totalNotes = 0;
        for (int e = 0; e < track->getNumEvents(); e++) {
            auto& msg = track->getEventPointer(e)->message;
            if (msg.isNoteOn()) {
                totalNotes++;
                if (msg.getChannel() == 10)
                    ch10Count++;
            }
        }

        if (ch10Count > 0 && !foundCh10) {
            foundCh10 = true;
            bestTrack = t;
            bestCount = ch10Count;
        } else if (foundCh10 && ch10Count > bestCount) {
            bestTrack = t;
            bestCount = ch10Count;
        } else if (!foundCh10 && totalNotes > bestCount) {
            bestTrack = t;
            bestCount = totalNotes;
        }
    }

    if (bestTrack < 0 || bestCount == 0) return pattern;

    double secondsPerBeat = 60.0 / bpm;
    double secondsPerStep = secondsPerBeat / settings.stepsPerBeat;
    int maxSteps = settings.maxBars * settings.stepsPerBeat * settings.beatsPerBar;

    // key: (step, midiNote) -> highest velocity
    std::map<std::pair<int, int>, float> hitMap;
    int maxStep = 0;

    auto* track = midiFile.getTrack(bestTrack);
    for (int e = 0; e < track->getNumEvents(); e++) {
        auto& msg = track->getEventPointer(e)->message;
        if (!msg.isNoteOn() || msg.getFloatVelocity() == 0.0f) continue;

        // If we found ch10, only use ch10 events
        if (foundCh10 && msg.getChannel() != 10) continue;

        int quantizedStep = (int)std::round(msg.getTimeStamp() / secondsPerStep);
        if (quantizedStep < 0) quantizedStep = 0;
        if (quantizedStep >= maxSteps) continue;

        int note = msg.getNoteNumber();
        float vel = msg.getFloatVelocity();

        auto key = std::make_pair(quantizedStep, note);
        auto it = hitMap.find(key);
        if (it == hitMap.end() || vel > it->second)
            hitMap[key] = vel;

        if (quantizedStep > maxStep)
            maxStep = quantizedStep;
    }

    // Round up to full bar boundary
    int stepsPerBar = settings.stepsPerBeat * settings.beatsPerBar;
    int numSteps = ((maxStep / stepsPerBar) + 1) * stepsPerBar;
    if (numSteps > maxSteps) numSteps = maxSteps;
    pattern.numSteps = numSteps;

    // Convert map to hits
    for (auto& [key, vel] : hitMap) {
        pattern.hits.push_back({key.first, DrumType::Kick, vel, key.second});
    }

    return pattern;
}
