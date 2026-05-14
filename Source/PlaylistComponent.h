#pragma once

#include <JuceHeader.h>
#include <vector>

class DJAudioPlayer;

class PlaylistComponent : public juce::Component,
    public juce::TableListBoxModel,
    public juce::Button::Listener
{
public:
    PlaylistComponent(DJAudioPlayer& deckA, DJAudioPlayer& deckB);
    ~PlaylistComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Table model
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber,
        int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId,
        int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId,
        bool isRowSelected,
        juce::Component* existingComponentToUpdate) override;

    void buttonClicked(juce::Button* button) override;

    // Playlist operations
    void addFiles();
    void clear();
    void saveToFile();
    void loadFromFile();

private:
    DJAudioPlayer& deckAPlayer;
    DJAudioPlayer& deckBPlayer;

    juce::TableListBox table{ "Playlist", this };
    juce::TextButton addButton{ "Add Tracks" };
    juce::TextButton saveButton{ "Save Playlist" };
    juce::TextButton loadButton{ "Load Playlist" };
    juce::TextButton clearButton{ "Clear" };

    std::vector<juce::File> files;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlaylistComponent)
};