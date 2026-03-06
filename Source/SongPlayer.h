#pragma once

#include "PatternData.h"

class DrumEngine;

class SongPlayer {
public:
    enum class State {
        Stopped,
        PlayingIntro,
        PlayingMain,
        PlayingFill,
        PlayingTransition,
        PlayingOutro
    };

    void prepare(double sampleRate);
    void process(int numSamples, DrumEngine& engine);

    void loadSong(const Song& song);
    void primaryAction();      // stopped → start; playing → fill
    void transitionAction();   // transition to next part
    void stopAction();         // play outro then stop

    void setTempo(float bpm);
    float getTempo() const { return tempo; }
    State getState() const { return state; }
    bool isPlaying() const { return state != State::Stopped; }
    int getCurrentStep() const { return currentStep; }
    int getCurrentPartIndex() const { return currentPartIndex; }
    int getTotalSteps() const { return currentPattern ? currentPattern->numSteps : 0; }
    int getCurrentBeat() const;
    int getBeatsPerBar() const;
    std::string getCurrentPartName() const;
    std::string getSongName() const { return currentSong.name; }

private:
    State state = State::Stopped;
    Song currentSong;
    const Pattern* currentPattern = nullptr;
    int currentPartIndex = 0;
    int currentStep = 0;
    float tempo = 120.0f;
    double sampleRate = 44100.0;
    double sampleCounter = 0.0;
    double nextStepTime = 0.0;

    bool fillRequested = false;
    bool transitionRequested = false;
    bool stopRequested = false;

    void startPattern(const Pattern* pattern, State newState);
    void handlePatternEnd();
    void triggerHitsAtStep(int step, DrumEngine& engine, int sampleOffset);
    double samplesPerStep() const;
};
