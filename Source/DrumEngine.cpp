#include "DrumEngine.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void DrumEngine::prepare(double sampleRate, int /*blockSize*/) {
    currentSampleRate = sampleRate;
    synthesizeAllSounds();

    // Panning: slight stereo spread
    setPan(DrumType::Kick, 0.0f);
    setPan(DrumType::Snare, 0.0f);
    setPan(DrumType::ClosedHH, 0.3f);
    setPan(DrumType::OpenHH, 0.35f);
    setPan(DrumType::Clap, -0.05f);
    setPan(DrumType::LowTom, -0.3f);
    setPan(DrumType::Rimshot, 0.1f);
    setPan(DrumType::Ride, 0.4f);
}

void DrumEngine::setPan(DrumType drum, float pan) {
    auto& v = voices[(int)drum];
    // Equal-power panning
    float angle = (pan + 1.0f) * 0.25f * (float)M_PI;  // 0 to π/2
    v.panL = std::cos(angle);
    v.panR = std::sin(angle);
}

void DrumEngine::trigger(DrumType drum, float velocity, int sampleOffset) {
    auto& v = voices[(int)drum];
    v.position = 0;
    v.velocity = velocity;
    v.blockStartOffset = sampleOffset;
    v.playing = true;
    v.justTriggered = true;
}

void DrumEngine::renderBlock(juce::AudioBuffer<float>& buffer) {
    int numSamples = buffer.getNumSamples();
    int numChannels = buffer.getNumChannels();

    for (auto& v : voices) {
        if (!v.playing) continue;

        int start = v.justTriggered ? v.blockStartOffset : 0;
        v.justTriggered = false;

        int sampleLen = v.sample.getNumSamples();
        const float* src = v.sample.getReadPointer(0);

        for (int i = start; i < numSamples; i++) {
            if (v.position >= sampleLen) {
                v.playing = false;
                break;
            }
            float s = src[v.position++] * v.velocity;
            buffer.addSample(0, i, s * v.panL);
            if (numChannels > 1)
                buffer.addSample(1, i, s * v.panR);
        }
    }
}

// ─── Drum synthesis ───────────────────────────────────────────

void DrumEngine::synthesizeAllSounds() {
    synthesizeKick(voices[(int)DrumType::Kick].sample);
    synthesizeSnare(voices[(int)DrumType::Snare].sample);
    synthesizeClosedHH(voices[(int)DrumType::ClosedHH].sample);
    synthesizeOpenHH(voices[(int)DrumType::OpenHH].sample);
    synthesizeClap(voices[(int)DrumType::Clap].sample);
    synthesizeLowTom(voices[(int)DrumType::LowTom].sample);
    synthesizeRimshot(voices[(int)DrumType::Rimshot].sample);
    synthesizeRide(voices[(int)DrumType::Ride].sample);
}

void DrumEngine::synthesizeKick(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.45);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;
    double fStart = 150.0, fEnd = 45.0, fDecay = 35.0;
    double ampDecay = 7.0;

    for (int i = 0; i < len; i++) {
        double t = i / sr;
        // Frequency sweep integral: ∫ f(t) dt where f(t) = fEnd + (fStart-fEnd)*e^(-t*fDecay)
        double phase = 2.0 * M_PI * (fEnd * t + (fStart - fEnd) / fDecay * (1.0 - std::exp(-t * fDecay)));
        double body = std::sin(phase) * std::exp(-t * ampDecay);

        // Transient click
        double click = std::exp(-t * 300.0) * 0.4 * (rng.nextFloat() * 2.0f - 1.0f);

        // Sub thump
        double sub = std::sin(2.0 * M_PI * 45.0 * t) * std::exp(-t * 12.0) * 0.3;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, body + click + sub));
    }
}

void DrumEngine::synthesizeSnare(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.35);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;

    for (int i = 0; i < len; i++) {
        double t = i / sr;

        // Tonal body
        double body = std::sin(2.0 * M_PI * 185.0 * t) * std::exp(-t * 20.0) * 0.5;

        // Noise (snare wires)
        double noise = (rng.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 12.0) * 0.6;

        // High-frequency snap
        double snap = (rng.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 50.0) * 0.3;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, body + noise + snap));
    }
}

void DrumEngine::synthesizeClosedHH(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.08);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;
    // Two square waves at inharmonic frequencies + noise, bandpassed
    double prevHP = 0.0;

    for (int i = 0; i < len; i++) {
        double t = i / sr;
        double amp = std::exp(-t * 80.0);

        // Metallic: sum of high-frequency oscillators
        double metal = std::sin(2.0 * M_PI * 3500.0 * t) * 0.3
                     + std::sin(2.0 * M_PI * 5100.0 * t) * 0.2
                     + std::sin(2.0 * M_PI * 7200.0 * t) * 0.15;

        double noise = (rng.nextFloat() * 2.0f - 1.0f) * 0.5;
        double raw = (metal + noise) * amp;

        // Simple high-pass: y[n] = x[n] - x[n-1] (crude but effective)
        double hp = raw - prevHP;
        prevHP = raw;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, hp * 0.8));
    }
}

void DrumEngine::synthesizeOpenHH(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.45);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;
    double prevHP = 0.0;

    for (int i = 0; i < len; i++) {
        double t = i / sr;
        double amp = std::exp(-t * 6.0);

        double metal = std::sin(2.0 * M_PI * 3500.0 * t) * 0.3
                     + std::sin(2.0 * M_PI * 5100.0 * t) * 0.2
                     + std::sin(2.0 * M_PI * 7200.0 * t) * 0.15;

        double noise = (rng.nextFloat() * 2.0f - 1.0f) * 0.5;
        double raw = (metal + noise) * amp;

        double hp = raw - prevHP;
        prevHP = raw;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, hp * 0.7));
    }
}

void DrumEngine::synthesizeClap(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.25);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;

    for (int i = 0; i < len; i++) {
        double t = i / sr;

        // Multiple noise bursts to simulate hand clap
        double env = 0.0;
        for (int b = 0; b < 4; b++) {
            double bt = t - b * 0.008;
            if (bt >= 0.0)
                env += std::exp(-bt * 100.0) * 0.3;
        }
        // Plus tail
        env += std::exp(-t * 15.0) * 0.4;

        double noise = (rng.nextFloat() * 2.0f - 1.0f);
        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, noise * env));
    }
}

void DrumEngine::synthesizeLowTom(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.5);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;
    double fStart = 200.0, fEnd = 80.0, fDecay = 20.0;

    for (int i = 0; i < len; i++) {
        double t = i / sr;
        double phase = 2.0 * M_PI * (fEnd * t + (fStart - fEnd) / fDecay * (1.0 - std::exp(-t * fDecay)));
        double body = std::sin(phase) * std::exp(-t * 6.0) * 0.8;

        double click = (rng.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 200.0) * 0.2;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, body + click));
    }
}

void DrumEngine::synthesizeRimshot(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 0.1);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;

    for (int i = 0; i < len; i++) {
        double t = i / sr;

        // Sharp click + ring
        double click = std::exp(-t * 400.0) * 0.6;
        double ring = std::sin(2.0 * M_PI * 1800.0 * t) * std::exp(-t * 60.0) * 0.5;
        double noise = (rng.nextFloat() * 2.0f - 1.0f) * std::exp(-t * 200.0) * 0.3;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, click + ring + noise));
    }
}

void DrumEngine::synthesizeRide(juce::AudioBuffer<float>& buf) {
    int len = (int)(currentSampleRate * 1.5);
    buf.setSize(1, len);
    buf.clear();

    double sr = currentSampleRate;
    double prevHP = 0.0;

    for (int i = 0; i < len; i++) {
        double t = i / sr;
        double amp = std::exp(-t * 2.0);

        // Richer metallic spectrum
        double metal = std::sin(2.0 * M_PI * 2800.0 * t) * 0.2
                     + std::sin(2.0 * M_PI * 4200.0 * t) * 0.15
                     + std::sin(2.0 * M_PI * 5800.0 * t) * 0.1
                     + std::sin(2.0 * M_PI * 8400.0 * t) * 0.05;

        double noise = (rng.nextFloat() * 2.0f - 1.0f) * 0.15;
        double raw = (metal + noise) * amp;

        // Gentle high-pass
        double hp = raw - 0.95 * prevHP;
        prevHP = raw;

        // Bell-like attack
        double attack = std::sin(2.0 * M_PI * 3000.0 * t) * std::exp(-t * 50.0) * 0.3;

        buf.setSample(0, i, (float)juce::jlimit(-1.0, 1.0, (hp + attack) * 0.6));
    }
}
