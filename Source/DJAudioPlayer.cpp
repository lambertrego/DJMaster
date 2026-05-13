#include "DJAudioPlayer.h"

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& fm) : formatManager(fm) {}

DJAudioPlayer::~DJAudioPlayer()
{
    transportSource.setSource(nullptr);
}

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);
}

void DJAudioPlayer::releaseResources()
{
    transportSource.releaseResources();
    resampleSource.releaseResources();
}

bool DJAudioPlayer::loadURL(const juce::URL& audioURL)
{
    auto stream = audioURL.createInputStream(false);
    if (stream == nullptr)
        return false;

    auto reader = std::unique_ptr<juce::AudioFormatReader>(
        formatManager.createReaderFor(std::move(stream)));
    if (reader == nullptr)
        return false;

    const auto sampleRate = reader->sampleRate;
    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
    transportSource.setSource(readerSource.get(), 0, nullptr, sampleRate);

    currentFile = audioURL.getLocalFile();          // NEW
    loadedTrackName = currentFile.getFileName();    // already present
    if (loadedTrackName.isEmpty())
        loadedTrackName = audioURL.toString(true);

    return true;
}

void DJAudioPlayer::start()
{
    transportSource.start();
}

void DJAudioPlayer::stop()
{
    transportSource.stop();
}

void DJAudioPlayer::setGain(double gain)
{
    transportSource.setGain((float)juce::jlimit(0.0, 1.0, gain));
}

void DJAudioPlayer::setSpeed(double ratio)
{
    resampleSource.setResamplingRatio(juce::jlimit(0.25, 4.0, ratio));
}

void DJAudioPlayer::setPosition(double positionInSeconds)
{
    transportSource.setPosition(positionInSeconds);
}

void DJAudioPlayer::setPositionRelative(double pos)
{
    auto length = getLengthInSeconds();
    if (length > 0.0)
        setPosition(pos * length);
}

void DJAudioPlayer::setVolume(double volume)
{
    setGain(volume);
}

double DJAudioPlayer::getPositionRelative() const
{
    const auto length = transportSource.getLengthInSeconds();
    if (length <= 0.0)
        return 0.0;

    return transportSource.getCurrentPosition() / length;
}

double DJAudioPlayer::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

bool DJAudioPlayer::isPlaying() const
{
    return transportSource.isPlaying();
}

juce::String DJAudioPlayer::getLoadedTrackName() const
{
    return loadedTrackName;
}