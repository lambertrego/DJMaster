#pragma once

#include <JuceHeader.h>
#include <memory>
 #include <juce_dsp/juce_dsp.h>  // if not already pulled in via JuceHeader

class DJAudioPlayer : public juce::AudioSource
{
public:
    explicit DJAudioPlayer(juce::AudioFormatManager& formatManager);
    ~DJAudioPlayer() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    bool loadURL(const juce::URL& audioURL);
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
    juce::String getLoadedTrackName() const;
    juce::File getCurrentFile() const { return currentFile; }
    void setLowPassCutoff(double cutoffHz);  // NEW
private:
    juce::AudioFormatManager& formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::ResamplingAudioSource resampleSource{ &transportSource, false, 2 };
    juce::String loadedTrackName{ "No track loaded" };
    juce::File currentFile;
    juce::dsp::IIR::Filter<float> lowPassFilter;   // NEW
    double lastSampleRate = 44100.0;               // NEW
    float lowPassCutoffHz = 20000.0f;              // NEW (default essentially bypass)
};