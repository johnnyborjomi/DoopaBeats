#include "DrumKitEditor.h"
#include "PluginProcessor.h"
#include "MidiNoteMap.h"

DrumKitEditor::DrumKitEditor(DoopaBeatsProcessor& p)
    : processor(p)
{
    kitSelector.onChange = [this] { onKitSelected(); };
    addAndMakeVisible(kitSelector);

    newButton.onClick = [this] { onNewKit(); };
    addAndMakeVisible(newButton);

    saveButton.onClick = [this] { onSaveKit(); };
    addAndMakeVisible(saveButton);

    deleteButton.onClick = [this] { onDeleteKit(); };
    addAndMakeVisible(deleteButton);

    kitNameEditor.setJustification(juce::Justification::centredLeft);
    addAndMakeVisible(kitNameEditor);

    viewport.setViewedComponent(&slotContainer, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    buildSlotRows();
    refreshKitList();
}

void DrumKitEditor::refreshKitList() {
    processor.refreshAvailableKits();
    kitSelector.clear(juce::dontSendNotification);
    kitSelector.addItem("Built-In", 1);

    auto kits = processor.getAvailableKits();
    for (int i = 0; i < kits.size(); i++)
        kitSelector.addItem(kits[i], i + 2);

    // Select current kit
    if (processor.isUsingBuiltInKit()) {
        kitSelector.setSelectedId(1, juce::dontSendNotification);
        kitNameEditor.setText("Built-In", false);
    } else {
        auto name = processor.getCurrentKitName();
        int idx = kits.indexOf(name);
        if (idx >= 0)
            kitSelector.setSelectedId(idx + 2, juce::dontSendNotification);
        kitNameEditor.setText(name, false);
    }

    updateSlotDisplay();
}

void DrumKitEditor::buildSlotRows() {
    slotRows.clear();
    auto notes = getDefaultSlotNotes();

    for (int note : notes) {
        SlotRow row;
        row.midiNote = note;

        row.noteLabel = std::make_unique<juce::Label>("", juce::String(note) + " " + midiNoteName(note));
        row.noteLabel->setFont(juce::FontOptions(13.0f));
        row.noteLabel->setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
        slotContainer.addAndMakeVisible(row.noteLabel.get());

        row.fileLabel = std::make_unique<juce::Label>("", "[empty]");
        row.fileLabel->setFont(juce::FontOptions(12.0f));
        row.fileLabel->setColour(juce::Label::textColourId, juce::Colour(0xffa0a0a0));
        slotContainer.addAndMakeVisible(row.fileLabel.get());

        row.loadButton = std::make_unique<juce::TextButton>("Load");
        row.loadButton->onClick = [this, note] { onLoadSample(note); };
        slotContainer.addAndMakeVisible(row.loadButton.get());

        row.clearButton = std::make_unique<juce::TextButton>("X");
        row.clearButton->onClick = [this, note] { onClearSample(note); };
        slotContainer.addAndMakeVisible(row.clearButton.get());

        slotRows.push_back(std::move(row));
    }
}

void DrumKitEditor::updateSlotDisplay() {
    auto& kit = processor.getCurrentKit();
    bool builtIn = processor.isUsingBuiltInKit();

    for (auto& row : slotRows) {
        auto* slot = kit.getSlot(row.midiNote);
        if (builtIn || !slot || !slot->loaded) {
            row.fileLabel->setText("[empty]", juce::dontSendNotification);
        } else {
            row.fileLabel->setText(slot->sourceFile.getFileName(), juce::dontSendNotification);
        }
        row.loadButton->setEnabled(!builtIn);
        row.clearButton->setEnabled(!builtIn && slot && slot->loaded);
    }
}

void DrumKitEditor::onKitSelected() {
    int id = kitSelector.getSelectedId();
    if (id == 1) {
        processor.loadBuiltInKit();
        kitNameEditor.setText("Built-In", false);
    } else {
        auto name = kitSelector.getText();
        processor.loadDrumKit(name);
        kitNameEditor.setText(name, false);
    }
    updateSlotDisplay();
}

void DrumKitEditor::onNewKit() {
    auto name = kitNameEditor.getText().trim();
    if (name.isEmpty() || name == "Built-In")
        name = "New Kit";

    DrumKit newKit;
    newKit.setName(name);
    auto kitFile = DrumKit::getKitsDirectory().getChildFile(name + ".doopakit");
    newKit.saveToFile(kitFile);

    processor.loadDrumKit(name);
    refreshKitList();
}

void DrumKitEditor::onSaveKit() {
    if (processor.isUsingBuiltInKit()) return;

    auto name = kitNameEditor.getText().trim();
    if (name.isEmpty() || name == "Built-In") return;

    auto& kit = processor.getCurrentKit();
    kit.setName(name);
    auto kitFile = DrumKit::getKitsDirectory().getChildFile(name + ".doopakit");
    kit.saveToFile(kitFile);
    refreshKitList();
}

void DrumKitEditor::onDeleteKit() {
    if (processor.isUsingBuiltInKit()) return;

    auto name = processor.getCurrentKitName();
    auto kitFile = DrumKit::getKitsDirectory().getChildFile(name + ".doopakit");
    kitFile.deleteFile();

    processor.loadBuiltInKit();
    refreshKitList();
}

void DrumKitEditor::onLoadSample(int midiNote) {
    if (processor.isUsingBuiltInKit()) return;

    auto chooser = std::make_shared<juce::FileChooser>(
        "Select a WAV sample",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.flac");

    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, midiNote, chooser](const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (!file.existsAsFile()) return;

            auto& kit = processor.getCurrentKit();
            double sr = processor.getSampleRate();
            if (sr <= 0.0) sr = 44100.0;

            if (kit.loadSample(midiNote, file, sr)) {
                processor.getDrumEngine().loadKit(kit);
                updateSlotDisplay();
            }
        });
}

void DrumKitEditor::onClearSample(int midiNote) {
    if (processor.isUsingBuiltInKit()) return;

    auto& kit = processor.getCurrentKit();
    kit.clearSlot(midiNote);
    processor.getDrumEngine().loadKit(kit);
    updateSlotDisplay();
}

void DrumKitEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1a1a2e));
}

void DrumKitEditor::resized() {
    auto bounds = getLocalBounds().reduced(8);

    // Top row: kit selector
    auto topRow = bounds.removeFromTop(30);
    kitSelector.setBounds(topRow);
    bounds.removeFromTop(6);

    // Second row: name editor + buttons
    auto row2 = bounds.removeFromTop(28);
    kitNameEditor.setBounds(row2.removeFromLeft(row2.getWidth() / 2 - 4));
    row2.removeFromLeft(8);
    int btnW = (row2.getWidth() - 8) / 3;
    newButton.setBounds(row2.removeFromLeft(btnW));
    row2.removeFromLeft(4);
    saveButton.setBounds(row2.removeFromLeft(btnW));
    row2.removeFromLeft(4);
    deleteButton.setBounds(row2);
    bounds.removeFromTop(8);

    // Slot list in viewport
    viewport.setBounds(bounds);

    int rowH = 30;
    int totalH = (int)slotRows.size() * rowH;
    slotContainer.setBounds(0, 0, bounds.getWidth() - 12, totalH);

    int w = slotContainer.getWidth();
    for (int i = 0; i < (int)slotRows.size(); i++) {
        auto& row = slotRows[i];
        int y = i * rowH;
        row.noteLabel->setBounds(0, y, 120, rowH);
        row.loadButton->setBounds(w - 70, y + 2, 40, rowH - 4);
        row.clearButton->setBounds(w - 26, y + 2, 24, rowH - 4);
        row.fileLabel->setBounds(122, y, w - 200, rowH);
    }
}
