#include "DrumKit.h"

DrumKit::DrumKit() {
    for (int note : getDefaultSlotNotes()) {
        DrumSlot slot;
        slot.midiNote = note;
        slot.name = midiNoteName(note);
        slots[note] = std::move(slot);
    }
}

DrumSlot* DrumKit::getSlot(int midiNote) {
    auto it = slots.find(midiNote);
    return it != slots.end() ? &it->second : nullptr;
}

const DrumSlot* DrumKit::getSlot(int midiNote) const {
    auto it = slots.find(midiNote);
    return it != slots.end() ? &it->second : nullptr;
}

bool DrumKit::loadSample(int midiNote, const juce::File& wavFile, double targetSampleRate) {
    auto* slot = getSlot(midiNote);
    if (!slot) return false;

    auto buf = readAndResample(wavFile, targetSampleRate);
    if (buf.getNumSamples() == 0) return false;

    // Copy WAV to samples directory for portability
    auto samplesDir = getSamplesDirectory();
    samplesDir.createDirectory();
    auto destFile = samplesDir.getChildFile(wavFile.getFileName());
    if (destFile != wavFile)
        wavFile.copyFileTo(destFile);

    slot->sample = std::move(buf);
    slot->sourceFile = destFile;
    slot->loaded = true;
    return true;
}

void DrumKit::clearSlot(int midiNote) {
    auto* slot = getSlot(midiNote);
    if (!slot) return;
    slot->sample.setSize(0, 0);
    slot->sourceFile = juce::File();
    slot->loaded = false;
}

juce::AudioBuffer<float> DrumKit::readAndResample(const juce::File& file, double targetSampleRate) {
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (!reader) return {};

    int numSamples = (int)reader->lengthInSamples;
    int numChannels = (int)reader->numChannels;
    double fileSampleRate = reader->sampleRate;

    // Read the file
    juce::AudioBuffer<float> fileBuffer(numChannels, numSamples);
    reader->read(&fileBuffer, 0, numSamples, 0, true, true);

    // Mix down to mono
    juce::AudioBuffer<float> mono(1, numSamples);
    mono.clear();
    for (int ch = 0; ch < numChannels; ch++)
        mono.addFrom(0, 0, fileBuffer, ch, 0, numSamples, 1.0f / numChannels);

    // Resample if needed
    if (std::abs(fileSampleRate - targetSampleRate) < 1.0)
        return mono;

    double ratio = fileSampleRate / targetSampleRate;
    int outLen = (int)(numSamples / ratio) + 1;
    juce::AudioBuffer<float> resampled(1, outLen);
    resampled.clear();

    juce::LagrangeInterpolator interpolator;
    int produced = interpolator.process(ratio,
                                         mono.getReadPointer(0),
                                         resampled.getWritePointer(0),
                                         outLen);
    resampled.setSize(1, produced, true);
    return resampled;
}

// ─── Persistence ─────────────────────────────────────────────

bool DrumKit::saveToFile(const juce::File& file) const {
    juce::ValueTree tree("DrumKit");
    tree.setProperty("name", kitName, nullptr);

    for (auto& [note, slot] : slots) {
        if (!slot.loaded) continue;
        juce::ValueTree slotTree("Slot");
        slotTree.setProperty("midiNote", note, nullptr);
        slotTree.setProperty("sampleFile", slot.sourceFile.getFullPathName(), nullptr);
        slotTree.setProperty("pan", (double)slot.pan, nullptr);
        slotTree.setProperty("gain", (double)slot.gain, nullptr);
        tree.addChild(slotTree, -1, nullptr);
    }

    auto xml = tree.createXml();
    if (!xml) return false;
    return xml->writeTo(file);
}

bool DrumKit::loadFromFile(const juce::File& file, double targetSampleRate) {
    auto xml = juce::XmlDocument::parse(file);
    if (!xml) return false;

    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid() || tree.getType().toString() != "DrumKit") return false;

    kitName = tree.getProperty("name", "Untitled Kit").toString();

    // Reset all slots
    for (auto& [note, slot] : slots) {
        slot.sample.setSize(0, 0);
        slot.sourceFile = juce::File();
        slot.loaded = false;
        slot.pan = 0.0f;
        slot.gain = 1.0f;
    }

    for (int i = 0; i < tree.getNumChildren(); i++) {
        auto slotTree = tree.getChild(i);
        int note = slotTree.getProperty("midiNote", -1);
        juce::String filePath = slotTree.getProperty("sampleFile", "").toString();
        float pan = (float)(double)slotTree.getProperty("pan", 0.0);
        float gain = (float)(double)slotTree.getProperty("gain", 1.0);

        auto* slot = getSlot(note);
        if (!slot) continue;

        juce::File sampleFile(filePath);
        if (sampleFile.existsAsFile()) {
            auto buf = readAndResample(sampleFile, targetSampleRate);
            if (buf.getNumSamples() > 0) {
                slot->sample = std::move(buf);
                slot->sourceFile = sampleFile;
                slot->loaded = true;
            }
        }
        slot->pan = pan;
        slot->gain = gain;
    }

    return true;
}

juce::File DrumKit::getKitsDirectory() {
    auto appDir = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory)
        .getChildFile("DoopaBeats")
        .getChildFile("DrumKits");
    appDir.createDirectory();
    return appDir;
}

juce::File DrumKit::getSamplesDirectory() {
    return getKitsDirectory().getChildFile("Samples");
}

juce::Array<juce::File> DrumKit::findAllKitFiles() {
    auto dir = getKitsDirectory();
    return dir.findChildFiles(juce::File::findFiles, false, "*.doopakit");
}
