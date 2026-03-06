cmake_minimum_required(VERSION 3.22)
project(DoopaBeats VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# JUCE — reuses the copy from doopaLooper_cmake
set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../doopaLooper_cmake/JUCE")
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_plugin(DoopaBeats
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE DpBt
    PLUGIN_CODE DBts
    FORMATS Standalone AUv3
    PRODUCT_NAME "DoopaBeats"
    BUNDLE_ID com.doopa.beats
    MICROPHONE_PERMISSION_ENABLED FALSE
)

juce_generate_juce_header(DoopaBeats)

target_sources(DoopaBeats PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DrumEngine.cpp
    Source/SongPlayer.cpp
    Source/DrumKit.cpp
    Source/DrumKitEditor.cpp
    Source/MidiImporter.cpp
    Source/UserSongLibrary.cpp
    Source/SongEditor.cpp
)

target_link_libraries(DoopaBeats PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
)

target_compile_definitions(DoopaBeats PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)
