#include "SongEditor.h"
#include "PluginProcessor.h"
#include "MidiImporter.h"
#include "MidiDrumMap.h"
#include "UserSongLibrary.h"

// Part index mapping:
// 0 = Intro, 1 = Verse (Main 1), 2 = Chorus (Main 2),
// 3 = Fill 1, 4 = Fill 2, 5 = Transition, 6 = Outro

static const char* partLabels[] = {
    "Intro", "Verse (Main 1)", "Chorus (Main 2)",
    "Fill 1", "Fill 2", "Transition", "Outro"
};
static constexpr int kNumParts = 7;

SongEditor::SongEditor(DoopaBeatsProcessor& p)
    : processor(p)
{
    songSelector.onChange = [this] { onSongSelected(); };
    addAndMakeVisible(songSelector);

    songNameEditor.setJustification(juce::Justification::centredLeft);
    addAndMakeVisible(songNameEditor);

    tempoSlider.setRange(40.0, 300.0, 1.0);
    tempoSlider.setValue(120.0, juce::dontSendNotification);
    tempoSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 24);
    tempoSlider.onValueChange = [this] {
        if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;
        auto& songs = processor.getSongs();
        songs[currentEditIndex].defaultTempo = (float)tempoSlider.getValue();
        processor.getSongPlayer().setTempo((float)tempoSlider.getValue());
    };
    addAndMakeVisible(tempoSlider);

    tempoLabel.setText("Tempo", juce::dontSendNotification);
    tempoLabel.setFont(juce::FontOptions(13.0f));
    tempoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    addAndMakeVisible(tempoLabel);

    newButton.onClick = [this] { onNewSong(); };
    addAndMakeVisible(newButton);

    saveButton.onClick = [this] { onSaveSong(); };
    addAndMakeVisible(saveButton);

    deleteButton.onClick = [this] { onDeleteSong(); };
    addAndMakeVisible(deleteButton);

    drumMapLabel.setText("MIDI Map:", juce::dontSendNotification);
    drumMapLabel.setFont(juce::FontOptions(13.0f));
    drumMapLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    addAndMakeVisible(drumMapLabel);

    auto presets = getPresetMaps();
    for (int i = 0; i < (int)presets.size(); i++)
        drumMapSelector.addItem(juce::String(presets[i].name), i + 1);
    drumMapSelector.setSelectedId(processor.getCurrentDrumMapIndex() + 1, juce::dontSendNotification);
    drumMapSelector.onChange = [this] {
        processor.setDrumMapIndex(drumMapSelector.getSelectedId() - 1);
    };
    addAndMakeVisible(drumMapSelector);

    viewport.setViewedComponent(&partContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    buildPartRows();
    refreshSongList();
}

void SongEditor::buildPartRows() {
    partRows.clear();
    for (int i = 0; i < kNumParts; i++) {
        PartRow row;
        row.label = partLabels[i];

        row.partLabel = std::make_unique<juce::Label>("", row.label);
        row.partLabel->setFont(juce::FontOptions(13.0f));
        row.partLabel->setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
        partContainer.addAndMakeVisible(row.partLabel.get());

        row.fileLabel = std::make_unique<juce::Label>("", "[empty]");
        row.fileLabel->setFont(juce::FontOptions(12.0f));
        row.fileLabel->setColour(juce::Label::textColourId, juce::Colour(0xffa0a0a0));
        partContainer.addAndMakeVisible(row.fileLabel.get());

        row.importButton = std::make_unique<juce::TextButton>("Import");
        row.importButton->onClick = [this, i] { onImportMidi(i); };
        partContainer.addAndMakeVisible(row.importButton.get());

        row.clearButton = std::make_unique<juce::TextButton>("X");
        row.clearButton->onClick = [this, i] { onClearPart(i); };
        partContainer.addAndMakeVisible(row.clearButton.get());

        partRows.push_back(std::move(row));
    }
}

void SongEditor::refreshSongList() {
    songSelector.clear(juce::dontSendNotification);
    auto& songs = processor.getSongs();
    int builtInCount = processor.getBuiltInSongCount();

    for (int i = 0; i < (int)songs.size(); i++) {
        juce::String prefix = (i >= builtInCount) ? "[User] " : "";
        songSelector.addItem(prefix + juce::String(songs[i].name), i + 1);

        // Add separator after built-in songs
        if (i == builtInCount - 1 && (int)songs.size() > builtInCount)
            songSelector.addSeparator();
    }

    if (currentEditIndex >= 0 && currentEditIndex < (int)songs.size())
        songSelector.setSelectedId(currentEditIndex + 1, juce::dontSendNotification);

    updatePartDisplay();
}

void SongEditor::syncToCurrentSong(int songIndex) {
    currentEditIndex = songIndex;
    if (songIndex >= 0 && songIndex < (int)processor.getSongs().size())
        songSelector.setSelectedId(songIndex + 1, juce::dontSendNotification);
    updatePartDisplay();
}

void SongEditor::updatePartDisplay() {
    auto& songs = processor.getSongs();
    bool isUser = (currentEditIndex >= 0 && processor.isUserSong(currentEditIndex));

    Song* song = nullptr;
    if (currentEditIndex >= 0 && currentEditIndex < (int)songs.size())
        song = &songs[currentEditIndex];

    songNameEditor.setEnabled(isUser);
    tempoSlider.setEnabled(isUser);
    saveButton.setEnabled(isUser);
    deleteButton.setEnabled(isUser);

    if (song) {
        songNameEditor.setText(juce::String(song->name), false);
        tempoSlider.setValue(song->defaultTempo, juce::dontSendNotification);
    }

    for (int i = 0; i < kNumParts; i++) {
        auto& row = partRows[i];
        bool enabled = isUser;
        row.importButton->setEnabled(enabled);

        if (song) {
            Pattern* pat = getPatternSlot(*song, i);
            bool hasHits = pat && !pat->hits.empty();
            row.fileLabel->setText(hasHits ? juce::String(pat->name) + " (" + juce::String((int)pat->hits.size()) + " hits)" : "[empty]",
                                   juce::dontSendNotification);
            row.clearButton->setEnabled(enabled && hasHits);
        } else {
            row.fileLabel->setText("[empty]", juce::dontSendNotification);
            row.clearButton->setEnabled(false);
        }
    }
}

Pattern* SongEditor::getPatternSlot(Song& song, int partIndex) {
    switch (partIndex) {
        case 0: return &song.intro;
        case 1: return song.mainLoops.size() > 0 ? &song.mainLoops[0] : nullptr;
        case 2: return song.mainLoops.size() > 1 ? &song.mainLoops[1] : nullptr;
        case 3: return song.fills.size() > 0 ? &song.fills[0] : nullptr;
        case 4: return song.fills.size() > 1 ? &song.fills[1] : nullptr;
        case 5: return song.transitions.size() > 0 ? &song.transitions[0] : nullptr;
        case 6: return &song.outro;
        default: return nullptr;
    }
}

void SongEditor::applyPatternToSlot(Song& song, int partIndex, const Pattern& pattern) {
    switch (partIndex) {
        case 0: // Intro
            song.intro = pattern;
            song.hasIntro = !pattern.hits.empty();
            break;
        case 1: // Verse (Main 1)
            if (song.mainLoops.empty())
                song.mainLoops.push_back(pattern);
            else
                song.mainLoops[0] = pattern;
            break;
        case 2: // Chorus (Main 2)
            while (song.mainLoops.size() < 2)
                song.mainLoops.push_back(Pattern{});
            song.mainLoops[1] = pattern;
            break;
        case 3: // Fill 1
            if (song.fills.empty())
                song.fills.push_back(pattern);
            else
                song.fills[0] = pattern;
            break;
        case 4: // Fill 2
            while (song.fills.size() < 2)
                song.fills.push_back(Pattern{});
            song.fills[1] = pattern;
            break;
        case 5: // Transition
            if (song.transitions.empty())
                song.transitions.push_back(pattern);
            else
                song.transitions[0] = pattern;
            break;
        case 6: // Outro
            song.outro = pattern;
            song.hasOutro = !pattern.hits.empty();
            break;
    }
}

// ─── Actions ──────────────────────────────────────────────────

void SongEditor::onSongSelected() {
    int idx = songSelector.getSelectedId() - 1;
    if (idx < 0) return;
    currentEditIndex = idx;
    processor.loadSong(idx);
    updatePartDisplay();
}

void SongEditor::onNewSong() {
    Song song;
    song.name = "New Song";
    song.defaultTempo = 120.0f;
    // Ensure at least one main loop so SongPlayer can start
    song.mainLoops.push_back(Pattern{"Main", 16, 4, 4, {}});

    processor.addUserSong(song);
    currentEditIndex = (int)processor.getSongs().size() - 1;
    processor.loadSong(currentEditIndex);
    refreshSongList();
}

void SongEditor::onSaveSong() {
    if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;

    auto& songs = processor.getSongs();
    auto& song = songs[currentEditIndex];

    // Apply name from editor
    auto name = songNameEditor.getText().trim();
    if (name.isNotEmpty())
        song.name = name.toStdString();

    auto file = UserSongLibrary::getSongsDirectory()
        .getChildFile(juce::String(song.name) + ".doopasong");
    UserSongLibrary::saveSong(song, file);
    refreshSongList();
}

void SongEditor::onDeleteSong() {
    if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;

    auto& songs = processor.getSongs();
    auto& song = songs[currentEditIndex];

    // Delete file
    auto file = UserSongLibrary::getSongsDirectory()
        .getChildFile(juce::String(song.name) + ".doopasong");
    file.deleteFile();

    processor.removeUserSong(currentEditIndex);
    currentEditIndex = 0;
    processor.loadSong(0);
    refreshSongList();
}

void SongEditor::onImportMidi(int partIndex) {
    if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;

    auto chooser = std::make_shared<juce::FileChooser>(
        "Select a MIDI drum pattern",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.mid;*.midi");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, partIndex, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (!file.existsAsFile()) return;
            if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;

            auto& songs = processor.getSongs();
            auto& song = songs[currentEditIndex];

            ImportSettings settings;
            settings.drumMap = &processor.getCurrentDrumMap();
            Pattern pattern = MidiImporter::importFromFile(file, settings);
            if (pattern.hits.empty()) return;

            applyPatternToSlot(song, partIndex, pattern);

            // Reload song into player so changes take effect immediately
            processor.loadSong(currentEditIndex);
            updatePartDisplay();
        });
}

void SongEditor::onClearPart(int partIndex) {
    if (currentEditIndex < 0 || !processor.isUserSong(currentEditIndex)) return;

    auto& songs = processor.getSongs();
    auto& song = songs[currentEditIndex];

    Pattern empty;
    empty.name = "";
    applyPatternToSlot(song, partIndex, empty);

    processor.loadSong(currentEditIndex);
    updatePartDisplay();
}

// ─── Paint / Layout ───────────────────────────────────────────

void SongEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a2e));
}

void SongEditor::resized() {
    auto bounds = getLocalBounds().reduced(8);

    // Song selector
    auto topRow = bounds.removeFromTop(30);
    songSelector.setBounds(topRow);
    bounds.removeFromTop(6);

    // Name editor + buttons
    auto row2 = bounds.removeFromTop(28);
    songNameEditor.setBounds(row2.removeFromLeft(row2.getWidth() / 2 - 4));
    row2.removeFromLeft(8);
    int btnW = (row2.getWidth() - 8) / 3;
    newButton.setBounds(row2.removeFromLeft(btnW));
    row2.removeFromLeft(4);
    saveButton.setBounds(row2.removeFromLeft(btnW));
    row2.removeFromLeft(4);
    deleteButton.setBounds(row2);
    bounds.removeFromTop(6);

    // Tempo row
    auto tempoRow = bounds.removeFromTop(28);
    tempoLabel.setBounds(tempoRow.removeFromLeft(50));
    tempoSlider.setBounds(tempoRow);
    bounds.removeFromTop(8);

    // Drum map selector row
    auto mapRow = bounds.removeFromTop(28);
    drumMapLabel.setBounds(mapRow.removeFromLeft(70));
    drumMapSelector.setBounds(mapRow);
    bounds.removeFromTop(8);

    // Part grid in viewport
    viewport.setBounds(bounds);

    int rowH = 36;
    int totalH = kNumParts * rowH;
    partContainer.setBounds(0, 0, bounds.getWidth() - 12, totalH);

    int w = partContainer.getWidth();
    for (int i = 0; i < (int)partRows.size(); i++) {
        auto& row = partRows[i];
        int y = i * rowH;
        row.partLabel->setBounds(0, y, 130, rowH);
        row.importButton->setBounds(w - 80, y + 2, 50, rowH - 4);
        row.clearButton->setBounds(w - 26, y + 2, 24, rowH - 4);
        row.fileLabel->setBounds(132, y, w - 220, rowH);
    }
}
