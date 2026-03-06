#pragma once

#include <JuceHeader.h>
#include "PatternData.h"

class DoopaBeatsProcessor;

class SongEditor : public juce::Component {
public:
    explicit SongEditor(DoopaBeatsProcessor& processor);
    ~SongEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshSongList();

    // Called by editor when user changes song in the main combo
    void syncToCurrentSong(int songIndex);

private:
    DoopaBeatsProcessor& processor;

    juce::ComboBox songSelector;
    juce::TextEditor songNameEditor;
    juce::Slider tempoSlider;
    juce::Label tempoLabel;
    juce::TextButton newButton    { "NEW" };
    juce::TextButton saveButton   { "SAVE" };
    juce::TextButton deleteButton { "DELETE" };

    struct PartRow {
        juce::String label;
        std::unique_ptr<juce::Label> partLabel;
        std::unique_ptr<juce::Label> fileLabel;
        std::unique_ptr<juce::TextButton> importButton;
        std::unique_ptr<juce::TextButton> clearButton;
    };

    std::vector<PartRow> partRows;
    juce::Viewport viewport;
    juce::Component partContainer;

    int currentEditIndex = -1;  // index in processor.getSongs()

    void buildPartRows();
    void updatePartDisplay();
    void onSongSelected();
    void onNewSong();
    void onSaveSong();
    void onDeleteSong();
    void onImportMidi(int partIndex);
    void onClearPart(int partIndex);

    Pattern* getPatternSlot(Song& song, int partIndex);
    void applyPatternToSlot(Song& song, int partIndex, const Pattern& pattern);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongEditor)
};
