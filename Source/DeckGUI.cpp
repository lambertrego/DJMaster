#include "DeckGUI.h"
#include <memory>

DeckGUI::DeckGUI(DJAudioPlayer& playerToControl, 
    juce::AudioFormatManager& fm,
    const juce::String& deckName)
    : player(playerToControl), formatManager(fm), looper(playerToControl)
{
    addAndMakeVisible(deckTitle);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(speedLabel);
    addAndMakeVisible(positionLabel);

    addAndMakeVisible(loadButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(loopButton);
    addAndMakeVisible(gainSlider);
    addAndMakeVisible(speedSlider);
    addAndMakeVisible(positionSlider);

    deckTitle.setText(deckName, juce::dontSendNotification);
    deckTitle.setJustificationType(juce::Justification::centred);
    deckTitle.setFont(juce::Font(18.0f, juce::Font::bold));

    gainLabel.setText("Gain", juce::dontSendNotification);
    speedLabel.setText("Speed", juce::dontSendNotification);
    positionLabel.setText("Position", juce::dontSendNotification);

    loadButton.addListener(this);
    playButton.addListener(this);
    stopButton.addListener(this);
    loopButton.addListener(this);

    for (auto* slider : { &gainSlider, &speedSlider, &positionSlider })
    {
        slider->addListener(this);
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 20);
    }

    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.8);
    speedSlider.setRange(0.25, 4.0, 0.01);
    speedSlider.setValue(1.0);
    positionSlider.setRange(0.0, 1.0, 0.001);

    startTimerHz(20);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    g.setColour(juce::Colour(0xff29434e));
    g.fillRoundedRectangle(area, 10.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(area.reduced(1.0f), 10.0f, 1.0f);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(12);

    deckTitle.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);

    auto buttonRow = area.removeFromTop(34);
    loadButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 3).reduced(2));
    playButton.setBounds(buttonRow.removeFromLeft(buttonRow.getWidth() / 2).reduced(2));
    stopButton.setBounds(buttonRow.reduced(2));

    area.removeFromTop(10);
    loopButton.setBounds(area.removeFromTop(24));

    area.removeFromTop(12);

    auto rowHeight = 30;

    auto gainRow = area.removeFromTop(rowHeight);
    gainLabel.setBounds(gainRow.removeFromLeft(70));
    gainSlider.setBounds(gainRow);

    area.removeFromTop(8);

    auto speedRow = area.removeFromTop(rowHeight);
    speedLabel.setBounds(speedRow.removeFromLeft(70));
    speedSlider.setBounds(speedRow);

    area.removeFromTop(8);

    auto positionRow = area.removeFromTop(rowHeight);
    positionLabel.setBounds(positionRow.removeFromLeft(70));
    positionSlider.setBounds(positionRow);
}

void DeckGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        auto chooser = std::make_shared<juce::FileChooser>("Select an audio file...");
        
        
        int flags = juce::FileBrowserComponent::openMode
                  | juce::FileBrowserComponent::canSelectFiles;
                  
        chooser->launchAsync(flags,
                             [this, chooser](const juce::FileChooser& fc)                            
                            {
                                auto file = fc.getResult();
                                if (file.existsAsFile())
                                    player.loadURL(juce::URL{ file });
                            });
    }
   
    else if (button == &playButton)
    {
        player.start();
    }
    else if (button == &stopButton)
    {
        player.stop();
    }
    else if (button == &loopButton)
    {
        looper.setEnabled(loopButton.getToggleState());
    }
}

void DeckGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &gainSlider)
        player.setGain(slider->getValue());
    else if (slider == &speedSlider)
        player.setSpeed(slider->getValue());
    else if (slider == &positionSlider)
        player.setPositionRelative(slider->getValue());
}

void DeckGUI::timerCallback()
{
    looper.update();

    if (! positionSlider.isMouseButtonDown())
        positionSlider.setValue(player.getPositionRelative(),
                                juce::dontSendNotification);
}