#include "SongPlayer.h"
#include "DrumEngine.h"

void SongPlayer::prepare(double sr) {
    sampleRate = sr;
}

void SongPlayer::loadSong(const Song& song) {
    state = State::Stopped;
    currentSong = song;
    tempo = song.defaultTempo;
    currentPartIndex = 0;
    currentStep = 0;
    currentPattern = nullptr;
    fillRequested = false;
    transitionRequested = false;
    stopRequested = false;
}

void SongPlayer::setTempo(float bpm) {
    tempo = juce::jlimit(40.0f, 300.0f, bpm);
}

double SongPlayer::samplesPerStep() const {
    if (!currentPattern || currentPattern->stepsPerBeat == 0) return sampleRate;
    double beatsPerSecond = tempo / 60.0;
    double stepsPerSecond = beatsPerSecond * currentPattern->stepsPerBeat;
    return sampleRate / stepsPerSecond;
}

int SongPlayer::getCurrentBeat() const {
    if (!currentPattern || currentPattern->stepsPerBeat == 0) return 0;
    int beat = currentStep / currentPattern->stepsPerBeat;
    return beat % currentPattern->beatsPerBar;
}

int SongPlayer::getBeatsPerBar() const {
    return currentPattern ? currentPattern->beatsPerBar : 4;
}

std::string SongPlayer::getCurrentPartName() const {
    switch (state) {
        case State::Stopped:           return "Stopped";
        case State::PlayingIntro:      return "Intro";
        case State::PlayingMain:       return currentPattern ? currentPattern->name : "Main";
        case State::PlayingFill:       return "Fill";
        case State::PlayingTransition: return "Transition";
        case State::PlayingOutro:      return "Outro";
    }
    return "";
}

// ─── Actions ──────────────────────────────────────────────────

void SongPlayer::primaryAction() {
    if (state == State::Stopped) {
        currentPartIndex = 0;
        if (currentSong.hasIntro && currentSong.intro.numSteps > 0) {
            startFromStopped(&currentSong.intro, State::PlayingIntro);
        } else if (!currentSong.mainLoops.empty()) {
            startFromStopped(&currentSong.mainLoops[0], State::PlayingMain);
        }
    } else {
        fillRequested = true;
    }
}

void SongPlayer::transitionAction() {
    if (state != State::Stopped)
        transitionRequested = true;
}

void SongPlayer::stopAction() {
    if (state == State::Stopped) return;
    if (currentSong.hasOutro && currentSong.outro.numSteps > 0)
        stopRequested = true;
    else
        state = State::Stopped;
}

// ─── Processing ───────────────────────────────────────────────

void SongPlayer::startPattern(const Pattern* pattern, State newState) {
    currentPattern = pattern;
    state = newState;
    currentStep = 0;
    // Don't reset sampleCounter/nextStepTime here — timing must be
    // continuous across pattern transitions to avoid an avalanche of
    // triggers in a single block.
}

void SongPlayer::startFromStopped(const Pattern* pattern, State newState) {
    sampleCounter = 0.0;
    nextStepTime = 0.0;
    startPattern(pattern, newState);
}

void SongPlayer::triggerHitsAtStep(int step, DrumEngine& engine, int sampleOffset) {
    if (!currentPattern) return;
    for (auto& hit : currentPattern->hits) {
        if (hit.step == step)
            engine.trigger(hit.drum, hit.velocity, sampleOffset);
    }
}

void SongPlayer::process(int numSamples, DrumEngine& engine) {
    if (state == State::Stopped || !currentPattern) return;

    double sps = samplesPerStep();
    double blockEnd = sampleCounter + numSamples;

    while (nextStepTime < blockEnd) {
        int offsetInBlock = (int)(nextStepTime - sampleCounter);
        if (offsetInBlock < 0) offsetInBlock = 0;
        if (offsetInBlock >= numSamples) break;

        triggerHitsAtStep(currentStep, engine, offsetInBlock);

        currentStep++;
        if (currentStep >= currentPattern->numSteps) {
            handlePatternEnd();
            if (state == State::Stopped) return;
            // After pattern change, recalculate sps in case stepsPerBeat changed
            sps = samplesPerStep();
        }

        nextStepTime += sps;
    }

    sampleCounter = blockEnd;
}

void SongPlayer::handlePatternEnd() {
    // Check stop request in any playing state (not just PlayingMain)
    if (stopRequested && state != State::PlayingOutro && state != State::Stopped) {
        stopRequested = false;
        fillRequested = false;
        transitionRequested = false;
        if (currentSong.hasOutro && currentSong.outro.numSteps > 0)
            startPattern(&currentSong.outro, State::PlayingOutro);
        else {
            state = State::Stopped;
            currentPattern = nullptr;
        }
        return;
    }

    switch (state) {
        case State::PlayingIntro:
            if (!currentSong.mainLoops.empty())
                startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
            else
                state = State::Stopped;
            break;

        case State::PlayingMain:
            if (transitionRequested) {
                transitionRequested = false;
                fillRequested = false;
                int ti = std::min(currentPartIndex, (int)currentSong.transitions.size() - 1);
                if (ti >= 0)
                    startPattern(&currentSong.transitions[ti], State::PlayingTransition);
                else {
                    currentPartIndex = (currentPartIndex + 1) % (int)currentSong.mainLoops.size();
                    startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
                }
            } else if (fillRequested) {
                fillRequested = false;
                int fi = std::min(currentPartIndex, (int)currentSong.fills.size() - 1);
                if (fi >= 0)
                    startPattern(&currentSong.fills[fi], State::PlayingFill);
                else
                    startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
            } else {
                startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
            }
            break;

        case State::PlayingFill:
            startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
            break;

        case State::PlayingTransition:
            currentPartIndex = (currentPartIndex + 1) % (int)currentSong.mainLoops.size();
            startPattern(&currentSong.mainLoops[currentPartIndex], State::PlayingMain);
            break;

        case State::PlayingOutro:
            state = State::Stopped;
            currentPattern = nullptr;
            break;

        case State::Stopped:
            break;
    }
}
