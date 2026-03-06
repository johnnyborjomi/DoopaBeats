#include "PluginProcessor.h"
#include "PluginEditor.h"

DoopaBeatsProcessor::DoopaBeatsProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    songs = BuiltInSongs::getAllSongs();
    if (!songs.empty())
        songPlayer.loadSong(songs[0]);
}

void DoopaBeatsProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    drumEngine.prepare(sampleRate, samplesPerBlock);
    songPlayer.prepare(sampleRate);
}

void DoopaBeatsProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    buffer.clear();
    songPlayer.process(buffer.getNumSamples(), drumEngine);
    drumEngine.renderBlock(buffer);

    // Soft-clip to prevent digital distortion from summed voices
    for (int ch = 0; ch < buffer.getNumChannels(); ch++) {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); i++) {
            data[i] = std::tanh(data[i] * 0.7f);
        }
    }
}

void DoopaBeatsProcessor::loadSong(int index) {
    if (index >= 0 && index < (int)songs.size()) {
        currentSongIndex = index;
        songPlayer.loadSong(songs[index]);
    }
}

juce::AudioProcessorEditor* DoopaBeatsProcessor::createEditor() {
    return new DoopaBeatsEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DoopaBeatsProcessor();
}
