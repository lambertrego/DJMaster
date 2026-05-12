#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : player1(formatManager),
    player2(formatManager),    
    deckGUI1(player1, formatManager, "Deck A"),
    deckGUI2(player2, formatManager, "Deck B")
{
    formatManager.registerBasicFormats();

    mixerSource.addInputSource(&player1, false);
    mixerSource.addInputSource(&player2, false);

    addAndMakeVisible(deckGUI1);
    addAndMakeVisible(deckGUI2);
    addAndMakeVisible(crossfader);
    addAndMakeVisible(titleLabel);

    titleLabel.setText("JUCE DJ Player", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));

    crossfader.setRange(0.0, 1.0, 0.01);
    crossfader.setValue(0.5);
    crossfader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    crossfader.onValueChange = [this]
        {
            const auto value = crossfader.getValue();
            player1.setVolume(1.0 - value);
            player2.setVolume(value);
        };

    crossfader.onValueChange();

    setSize(900, 500);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    mixerSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    mixerSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    mixerSource.releaseResources();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(12);

    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    crossfader.setBounds(area.removeFromTop(70));
    area.removeFromTop(10);

    auto deckArea = area;
    deckGUI1.setBounds(deckArea.removeFromLeft(deckArea.getWidth() / 2).reduced(6));
    deckGUI2.setBounds(deckArea.reduced(6));
}