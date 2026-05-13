#pragma once

#include <JuceHeader.h>
#include "DJAudioPlayer.h"
#include "Looper.h"

class DeckGUI : public juce::Component,
    public juce::Button::Listener,
    public juce::Slider::Listener,
    public juce::Timer
{
public:
    DeckGUI(DJAudioPlayer& playerToControl,
        juce::AudioFormatManager& formatManager,
        const juce::String& deckName);
    ~DeckGUI() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void timerCallback() override;

private:
    void drawAnimatedDeck(juce::Graphics& g, juce::Rectangle<float> area);
    void drawMarqueeText(juce::Graphics& g, juce::Rectangle<int> area);

    DJAudioPlayer& player;
    juce::AudioFormatManager& formatManager;
    Looper<DJAudioPlayer> looper;
    juce::AudioThumbnailCache thumbnailCache{ 64 };
    juce::AudioThumbnail thumbnail{ 512, formatManager, thumbnailCache };
    bool hasWaveform = false;

    juce::String deckName;
    juce::String marqueeText{ "No track loaded" };
    float marqueeOffset = 0.0f;
    float platterAngle = 0.0f;

    juce::Label deckTitle;
    juce::Label gainLabel;
    juce::Label speedLabel;
    juce::Label positionLabel;

    juce::TextButton loadButton{ "Load" };
    juce::TextButton playButton{ "Play" };
    juce::TextButton stopButton{ "Stop" };
    juce::ToggleButton loopButton{ "Loop" };

    juce::Slider gainSlider;
    juce::Slider speedSlider;
    juce::Slider positionSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeckGUI)
};