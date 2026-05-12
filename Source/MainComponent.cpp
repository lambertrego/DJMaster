#include "MainComponent.h"

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
    addAndMakeVisible(crossfaderLabel);

    titleLabel.setText("DJ Master", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    crossfaderLabel.setText("Crossfader", juce::dontSendNotification);
    crossfaderLabel.setJustificationType(juce::Justification::centred);
    crossfaderLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    crossfader.setRange(0.0, 1.0, 0.01);
    crossfader.setValue(0.5);
    crossfader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 22);
    crossfader.onValueChange = [this]
        {
            const auto value = crossfader.getValue();
            player1.setVolume(1.0 - value);
            player2.setVolume(value);
        };
    crossfader.onValueChange();

    setSize(1080, 640);
    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

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

void MainComponent::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(juce::Colour(0xff05070d), 0.0f, 0.0f,
        juce::Colour(0xff111827), 0.0f, (float)getHeight(), false);
    gradient.addColour(0.45, juce::Colour(0xff0b1220));
    gradient.addColour(0.75, juce::Colour(0xff09090f));
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(juce::Colour(0x22ffffff));
    g.drawLine(20.0f, 150.0f, (float)getWidth() - 20.0f, 150.0f, 1.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(16);

    titleLabel.setBounds(area.removeFromTop(42));
    crossfaderLabel.setBounds(area.removeFromTop(24));
    crossfader.setBounds(area.removeFromTop(70).reduced(40, 0));

    area.removeFromTop(14);
    auto decks = area;
    auto left = decks.removeFromLeft(decks.getWidth() / 2).reduced(6);
    auto right = decks.reduced(6);

    deckGUI1.setBounds(left);
    deckGUI2.setBounds(right);
}