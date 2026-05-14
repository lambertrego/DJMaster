#include "PlaylistComponent.h"
#include "DJAudioPlayer.h"

PlaylistComponent::PlaylistComponent(DJAudioPlayer& deckA,
    DJAudioPlayer& deckB)
    : deckAPlayer(deckA),
    deckBPlayer(deckB)
{
    table.getHeader().addColumn("Track", 1, 300);
    table.getHeader().addColumn("Deck A", 2, 70);
    table.getHeader().addColumn("Deck B", 3, 70);
    table.setModel(this);

    addAndMakeVisible(table);

    for (auto* b : { &addButton, &saveButton, &loadButton, &clearButton })
    {
        addAndMakeVisible(b);
        b->addListener(this);
    }
}

void PlaylistComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.8f));
}

void PlaylistComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    auto buttonRow = area.removeFromBottom(30);

    addButton.setBounds(buttonRow.removeFromLeft(100).reduced(2));
    saveButton.setBounds(buttonRow.removeFromLeft(110).reduced(2));
    loadButton.setBounds(buttonRow.removeFromLeft(110).reduced(2));
    clearButton.setBounds(buttonRow.removeFromLeft(80).reduced(2));

    table.setBounds(area);
}

// Table model
int PlaylistComponent::getNumRows()
{
    return (int)files.size();
}

void PlaylistComponent::paintRowBackground(juce::Graphics& g, int rowNumber,
    int width, int height,
    bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::darkcyan.withAlpha(0.5f));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colours::darkgrey.withAlpha(0.4f));
    else
        g.fillAll(juce::Colours::black.withAlpha(0.4f));
}

void PlaylistComponent::paintCell(juce::Graphics& g, int rowNumber,
    int columnId, int width, int height,
    bool rowIsSelected)
{
    if (columnId == 1 && rowNumber < (int)files.size())
    {
        g.setColour(juce::Colours::white);
        g.drawText(files[(size_t)rowNumber].getFileName(),
            2, 0, width - 4, height,
            juce::Justification::centredLeft, true);
    }
}

juce::Component* PlaylistComponent::refreshComponentForCell(int rowNumber,
    int columnId,
    bool isRowSelected,
    juce::Component* existingComponentToUpdate)
{
    if (columnId == 2 || columnId == 3)
    {
        auto* btn = dynamic_cast<juce::TextButton*>(existingComponentToUpdate);
        if (btn == nullptr)
        {
            btn = new juce::TextButton(columnId == 2 ? "Load" : "Load");
            btn->addListener(this);
        }

        btn->setComponentID(juce::String(columnId) + ":" + juce::String(rowNumber));
        return btn;
    }

    return existingComponentToUpdate;
}

void PlaylistComponent::buttonClicked(juce::Button* button)
{
    if (button == &addButton)
    {
        addFiles();
        return;
    }
    if (button == &saveButton)
    {
        saveToFile();
        return;
    }
    if (button == &loadButton)
    {
        loadFromFile();
        return;
    }
    if (button == &clearButton)
    {
        clear();
        return;
    }

    // Table buttons: componentID = "col:row"
    const auto id = button->getComponentID();
    const auto parts = juce::StringArray::fromTokens(id, ":", "");
    if (parts.size() == 2)
    {
        const int col = parts[0].getIntValue();
        const int row = parts[1].getIntValue();

        if (row >= 0 && row < (int)files.size())
        {
            auto file = files[(size_t)row];
            if (file.existsAsFile())
            {
                if (col == 2)
                    deckAPlayer.loadURL(juce::URL(file));
                else if (col == 3)
                    deckBPlayer.loadURL(juce::URL(file));
            }
        }
    }
}

// Playlist operations
void PlaylistComponent::addFiles()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select audio files...", juce::File(), "*");

    int flags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::canSelectMultipleItems;

    chooser->launchAsync(flags,
        [this, chooser](const juce::FileChooser& fc)
        {
            juce::Array<juce::File> results = fc.getResults();
            if (results.isEmpty())
                return; // user cancelled

            for (auto& f : results)
                files.push_back(f);

            table.updateContent();
            table.repaint();
        });
}

void PlaylistComponent::clear()
{
    files.clear();
    table.updateContent();
    table.repaint();
}

void PlaylistComponent::saveToFile()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Save playlist...", juce::File(), "*.djplaylist");

    int flags = juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.getFullPathName().isEmpty())
                return; // user cancelled

            juce::StringArray lines;
            for (auto& f : files)
                lines.add(f.getFullPathName());

            file.replaceWithText(lines.joinIntoString("\n"));
        });
}


void PlaylistComponent::loadFromFile()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Load playlist...", juce::File(), "*.djplaylist");

    int flags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(flags,
        [this, chooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (!file.existsAsFile())
                return; // user cancelled or file missing

            juce::StringArray lines;
            file.readLines(lines);

            files.clear();

            for (auto& path : lines)
            {
                juce::File f(path);
                if (f.existsAsFile())
                    files.push_back(f);
            }

            table.updateContent();
            table.repaint();
        });
}