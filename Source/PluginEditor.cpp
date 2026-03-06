#include "PluginEditor.h"

namespace Theme {
    static const juce::Colour bg        { 0xff1a1a2e };
    static const juce::Colour panel     { 0xff16213e };
    static const juce::Colour accent    { 0xff0f3460 };
    static const juce::Colour textLight { 0xffe0e0e0 };
    static const juce::Colour beatOff   { 0xff2a2a4a };
    static const juce::Colour beatOn    { 0xfff5a623 };
    static const juce::Colour tapGreen  { 0xff27ae60 };
    static const juce::Colour transBlue { 0xff2980b9 };
    static const juce::Colour stopRed   { 0xffc0392b };
}

DoopaBeatsEditor::DoopaBeatsEditor(DoopaBeatsProcessor& p)
    : AudioProcessorEditor(&p), processor(p), kitEditor(p)
{
    setSize(500, 720);

    // Song selector
    auto& songs = processor.getSongs();
    for (int i = 0; i < (int)songs.size(); i++)
        songSelector.addItem(songs[i].name, i + 1);
    songSelector.setSelectedId(1, juce::dontSendNotification);
    songSelector.onChange = [this] {
        int idx = songSelector.getSelectedId() - 1;
        processor.loadSong(idx);
    };
    addAndMakeVisible(songSelector);

    // Tempo slider
    tempoSlider.setRange(40.0, 300.0, 1.0);
    tempoSlider.setValue(processor.getSongPlayer().getTempo(), juce::dontSendNotification);
    tempoSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    tempoSlider.setColour(juce::Slider::thumbColourId, Theme::beatOn);
    tempoSlider.setColour(juce::Slider::trackColourId, Theme::accent);
    tempoSlider.onValueChange = [this] {
        processor.getSongPlayer().setTempo((float)tempoSlider.getValue());
    };
    addAndMakeVisible(tempoSlider);

    tempoLabel.setFont(juce::FontOptions(20.0f));
    tempoLabel.setColour(juce::Label::textColourId, Theme::textLight);
    tempoLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tempoLabel);

    // Buttons
    tapButton.setColour(juce::TextButton::buttonColourId, Theme::tapGreen);
    tapButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    tapButton.onClick = [this] { processor.getSongPlayer().primaryAction(); };
    addAndMakeVisible(tapButton);

    transButton.setColour(juce::TextButton::buttonColourId, Theme::transBlue);
    transButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    transButton.onClick = [this] { processor.getSongPlayer().transitionAction(); };
    addAndMakeVisible(transButton);

    stopButton.setColour(juce::TextButton::buttonColourId, Theme::stopRed);
    stopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    stopButton.onClick = [this] { processor.getSongPlayer().stopAction(); };
    addAndMakeVisible(stopButton);

    kitButton.setColour(juce::TextButton::buttonColourId, Theme::accent);
    kitButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    kitButton.onClick = [this] {
        showingKitView = !showingKitView;
        kitButton.setButtonText(showingKitView ? "PLAY" : "KIT");
        if (showingKitView)
            kitEditor.refreshKitList();
        kitEditor.setVisible(showingKitView);
        songSelector.setVisible(!showingKitView);
        tapButton.setVisible(!showingKitView);
        transButton.setVisible(!showingKitView);
        stopButton.setVisible(!showingKitView);
        tempoSlider.setVisible(!showingKitView);
        tempoLabel.setVisible(!showingKitView);
        resized();
        repaint();
    };
    addAndMakeVisible(kitButton);

    kitEditor.setVisible(false);
    addAndMakeVisible(kitEditor);

    startTimerHz(30);
}

void DoopaBeatsEditor::timerCallback() {
    // Update tempo display
    float currentTempo = processor.getSongPlayer().getTempo();
    tempoSlider.setValue(currentTempo, juce::dontSendNotification);
    tempoLabel.setText(juce::String((int)currentTempo) + " BPM", juce::dontSendNotification);

    // Update tap button text based on state
    auto state = processor.getSongPlayer().getState();
    if (state == SongPlayer::State::Stopped)
        tapButton.setButtonText("START");
    else
        tapButton.setButtonText("FILL");

    repaint();
}

void DoopaBeatsEditor::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.fillAll(Theme::bg);

    // ─── Title ─────────────────────────────────────────────
    g.setColour(Theme::textLight);
    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.drawText("DoopaBeats", bounds.removeFromTop(50), juce::Justification::centred);

    if (showingKitView) return;

    auto& player = processor.getSongPlayer();
    int currentBeat = player.getCurrentBeat();
    int beatsPerBar = player.getBeatsPerBar();
    bool playing = player.isPlaying();

    // ─── Part name ─────────────────────────────────────────
    auto partArea = bounds.withHeight(40).withY(140);
    g.setFont(juce::FontOptions(20.0f));
    g.setColour(Theme::textLight.withAlpha(0.8f));
    g.drawText(player.getCurrentPartName(), partArea, juce::Justification::centred);

    // ─── Beat indicators ───────────────────────────────────
    int beatAreaY = 190;
    int dotSize = 34;
    int totalDotsWidth = beatsPerBar * dotSize + (beatsPerBar - 1) * 14;
    int startX = (getWidth() - totalDotsWidth) / 2;

    for (int i = 0; i < beatsPerBar; i++) {
        int x = startX + i * (dotSize + 14);
        bool isActive = playing && (i == currentBeat);

        if (isActive) {
            g.setColour(Theme::beatOn.withAlpha(0.3f));
            g.fillEllipse((float)(x - 5), (float)(beatAreaY - 5),
                          (float)(dotSize + 10), (float)(dotSize + 10));
        }

        g.setColour(isActive ? Theme::beatOn : Theme::beatOff);
        g.fillEllipse((float)x, (float)beatAreaY, (float)dotSize, (float)dotSize);

        g.setColour(isActive ? juce::Colours::black : Theme::textLight.withAlpha(0.4f));
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.drawText(juce::String(i + 1),
                   x, beatAreaY, dotSize, dotSize,
                   juce::Justification::centred);
    }
}

void DoopaBeatsEditor::resized() {
    auto bounds = getLocalBounds().reduced(24);

    // Title area (50px) is drawn in paint
    auto topArea = bounds.removeFromTop(50);

    // KIT button in top-right corner
    kitButton.setBounds(topArea.removeFromRight(60).withHeight(30).withY(topArea.getY() + 10));

    // Song selector
    topArea.removeFromTop(10);
    songSelector.setBounds(topArea.removeFromTop(32).withTrimmedRight(8));

    if (showingKitView) {
        bounds.removeFromTop(8);
        kitEditor.setBounds(bounds);
    } else {
        bounds.removeFromTop(150); // space for part name + beat indicators

        // Tempo area
        auto tempoArea = bounds.removeFromTop(65);
        tempoLabel.setBounds(tempoArea.removeFromTop(28));
        tempoSlider.setBounds(tempoArea);

        bounds.removeFromTop(30);

        // Buttons
        int buttonHeight = 80;
        int buttonSpacing = 14;

        tapButton.setBounds(bounds.removeFromTop(buttonHeight));
        bounds.removeFromTop(buttonSpacing);

        auto bottomRow = bounds.removeFromTop(buttonHeight);
        int halfWidth = (bottomRow.getWidth() - buttonSpacing) / 2;
        transButton.setBounds(bottomRow.removeFromLeft(halfWidth));
        bottomRow.removeFromLeft(buttonSpacing);
        stopButton.setBounds(bottomRow);
    }
}
