#include "PluginProcessor.h"
#include "PluginEditor.h"

DoopaBeatsProcessor::DoopaBeatsProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    songs = BuiltInSongs::getAllSongs();
    if (!songs.empty())
        songPlayer.loadSong(songs[0]);

    refreshAvailableKits();
}

void DoopaBeatsProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    lastSampleRate = sampleRate;
    drumEngine.prepare(sampleRate, samplesPerBlock);
    songPlayer.prepare(sampleRate);

    // If using a user kit, reload samples at new sample rate
    if (!usingBuiltIn) {
        auto kitFile = DrumKit::getKitsDirectory().getChildFile(currentKitName + ".doopakit");
        if (kitFile.existsAsFile()) {
            currentKit.loadFromFile(kitFile, sampleRate);
            drumEngine.loadKit(currentKit);
        }
    }
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

void DoopaBeatsProcessor::loadDrumKit(const juce::String& kitName) {
    auto kitFile = DrumKit::getKitsDirectory().getChildFile(kitName + ".doopakit");
    if (!kitFile.existsAsFile()) return;

    if (currentKit.loadFromFile(kitFile, lastSampleRate)) {
        currentKitName = kitName;
        usingBuiltIn = false;
        drumEngine.loadKit(currentKit);
    }
}

void DoopaBeatsProcessor::loadBuiltInKit() {
    currentKitName = "Built-In";
    usingBuiltIn = true;
    drumEngine.loadBuiltInKit();
}

void DoopaBeatsProcessor::refreshAvailableKits() {
    availableKitNames.clear();
    for (auto& f : DrumKit::findAllKitFiles())
        availableKitNames.add(f.getFileNameWithoutExtension());
}

juce::StringArray DoopaBeatsProcessor::getAvailableKits() const {
    return availableKitNames;
}

void DoopaBeatsProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ValueTree state("DoopaBeatsState");
    state.setProperty("kitName", currentKitName, nullptr);
    state.setProperty("usingBuiltIn", usingBuiltIn, nullptr);
    state.setProperty("songIndex", currentSongIndex, nullptr);

    auto xml = state.createXml();
    if (xml)
        copyXmlToBinary(*xml, destData);
}

void DoopaBeatsProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (!xml) return;

    auto state = juce::ValueTree::fromXml(*xml);
    if (!state.isValid()) return;

    int songIdx = state.getProperty("songIndex", 0);
    loadSong(songIdx);

    bool builtIn = state.getProperty("usingBuiltIn", true);
    juce::String kitN = state.getProperty("kitName", "Built-In").toString();

    if (builtIn)
        loadBuiltInKit();
    else
        loadDrumKit(kitN);
}

juce::AudioProcessorEditor* DoopaBeatsProcessor::createEditor() {
    return new DoopaBeatsEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DoopaBeatsProcessor();
}
