#include "UserSongLibrary.h"

juce::File UserSongLibrary::getSongsDirectory() {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DoopaBeats")
        .getChildFile("Songs");
    dir.createDirectory();
    return dir;
}

juce::Array<juce::File> UserSongLibrary::findAllSongFiles() {
    return getSongsDirectory().findChildFiles(juce::File::findFiles, false, "*.doopasong");
}

static juce::ValueTree patternToTree(const Pattern& p, const juce::String& tag) {
    juce::ValueTree tree(tag);
    tree.setProperty("name", juce::String(p.name), nullptr);
    tree.setProperty("numSteps", p.numSteps, nullptr);
    tree.setProperty("stepsPerBeat", p.stepsPerBeat, nullptr);
    tree.setProperty("beatsPerBar", p.beatsPerBar, nullptr);

    for (auto& hit : p.hits) {
        juce::ValueTree h("Hit");
        h.setProperty("step", hit.step, nullptr);
        h.setProperty("midiNote", hit.midiNote, nullptr);
        h.setProperty("velocity", (double)hit.velocity, nullptr);
        tree.addChild(h, -1, nullptr);
    }
    return tree;
}

static Pattern treeToPattern(const juce::ValueTree& tree) {
    Pattern p;
    p.name = tree.getProperty("name", "").toString().toStdString();
    p.numSteps = tree.getProperty("numSteps", 16);
    p.stepsPerBeat = tree.getProperty("stepsPerBeat", 4);
    p.beatsPerBar = tree.getProperty("beatsPerBar", 4);

    for (int i = 0; i < tree.getNumChildren(); i++) {
        auto h = tree.getChild(i);
        if (h.getType().toString() != "Hit") continue;
        Hit hit;
        hit.step = h.getProperty("step", 0);
        hit.midiNote = h.getProperty("midiNote", 36);
        hit.velocity = (float)(double)h.getProperty("velocity", 1.0);
        hit.drum = DrumType::Kick;
        p.hits.push_back(hit);
    }
    return p;
}

bool UserSongLibrary::saveSong(const Song& song, const juce::File& file) {
    juce::ValueTree tree("DoopaBeatsong");
    tree.setProperty("name", juce::String(song.name), nullptr);
    tree.setProperty("defaultTempo", (double)song.defaultTempo, nullptr);
    tree.setProperty("hasIntro", song.hasIntro, nullptr);
    tree.setProperty("hasOutro", song.hasOutro, nullptr);

    tree.addChild(patternToTree(song.intro, "Intro"), -1, nullptr);
    tree.addChild(patternToTree(song.outro, "Outro"), -1, nullptr);

    for (auto& ml : song.mainLoops)
        tree.addChild(patternToTree(ml, "MainLoop"), -1, nullptr);
    for (auto& f : song.fills)
        tree.addChild(patternToTree(f, "Fill"), -1, nullptr);
    for (auto& t : song.transitions)
        tree.addChild(patternToTree(t, "Transition"), -1, nullptr);

    auto xml = tree.createXml();
    if (!xml) return false;
    return xml->writeTo(file);
}

Song UserSongLibrary::loadSong(const juce::File& file) {
    Song song;
    auto xml = juce::XmlDocument::parse(file);
    if (!xml) return song;

    auto tree = juce::ValueTree::fromXml(*xml);
    if (!tree.isValid() || tree.getType().toString() != "DoopaBeatsong") return song;

    song.name = tree.getProperty("name", "Untitled").toString().toStdString();
    song.defaultTempo = (float)(double)tree.getProperty("defaultTempo", 120.0);
    song.hasIntro = tree.getProperty("hasIntro", false);
    song.hasOutro = tree.getProperty("hasOutro", false);

    for (int i = 0; i < tree.getNumChildren(); i++) {
        auto child = tree.getChild(i);
        auto type = child.getType().toString();

        if (type == "Intro")         song.intro = treeToPattern(child);
        else if (type == "Outro")    song.outro = treeToPattern(child);
        else if (type == "MainLoop") song.mainLoops.push_back(treeToPattern(child));
        else if (type == "Fill")     song.fills.push_back(treeToPattern(child));
        else if (type == "Transition") song.transitions.push_back(treeToPattern(child));
    }

    return song;
}
