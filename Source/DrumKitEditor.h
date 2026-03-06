#pragma once

#include <JuceHeader.h>
#include "DrumKit.h"

class DoopaBeatsProcessor;

class DrumKitEditor : public juce::Component {
public:
    explicit DrumKitEditor(DoopaBeatsProcessor& processor);
    ~DrumKitEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    void refreshKitList();

private:
    DoopaBeatsProcessor& processor;

    juce::ComboBox kitSelector;
    juce::TextButton newButton   { "NEW" };
    juce::TextButton saveButton  { "SAVE" };
    juce::TextButton deleteButton { "DELETE" };
    juce::TextEditor kitNameEditor;

    struct SlotRow {
        int midiNote = 0;
        std::unique_ptr<juce::Label> noteLabel;
        std::unique_ptr<juce::Label> fileLabel;
        std::unique_ptr<juce::TextButton> loadButton;
        std::unique_ptr<juce::TextButton> clearButton;
    };

    std::vector<SlotRow> slotRows;
    juce::Viewport viewport;
    juce::Component slotContainer;

    void buildSlotRows();
    void updateSlotDisplay();
    void onKitSelected();
    void onNewKit();
    void onSaveKit();
    void onDeleteKit();
    void onLoadSample(int midiNote);
    void onClearSample(int midiNote);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumKitEditor)
};
