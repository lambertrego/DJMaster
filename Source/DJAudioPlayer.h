#pragma once

#include <JuceHeader.h>
#include <memory>

class DJAudioPlayer : public juce::AudioSource
{
public:
    DJAudioPlayer(juce::AudioFormatManager& formatManager);
    ~DJAudioPlayer() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void loadURL(const juce::URL& audioURL);
    void start();
    void stop();

    void setGain(double gain);
    void setSpeed(double ratio);
    void setPosition(double positionInSeconds);
    void setPositionRelative(double pos);
    void setVolume(double volume);

    double getPositionRelative() const;
    double getLengthInSeconds() const;
    bool isPlaying() const;

private:
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource { &transportSource, false, 2 };
};