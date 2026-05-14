#include "DJAudioPlayer.h"
#include <juce_dsp/juce_dsp.h> 

DJAudioPlayer::DJAudioPlayer(juce::AudioFormatManager& fm) : formatManager(fm) {}

DJAudioPlayer::~DJAudioPlayer()
{
    transportSource.setSource(nullptr);
}

void DJAudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampleSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec{ sampleRate,
                                  static_cast<juce::uint32>(samplesPerBlockExpected),
                                  2 };
    lowPassFilter.reset();
    lowPassFilter.prepare(spec);
    setLowPassCutoff(lowPassCutoffHz);  // initialise coefficients
}

void DJAudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    resampleSource.getNextAudioBlock(bufferToFill);

    auto* buffer = bufferToFill.buffer;
    if (buffer == nullptr)
        return;

    const int numChannels = buffer->getNumChannels();
    const int startSample = bufferToFill.startSample;
    const int numSamples = bufferToFill.numSamples;

    // Pointer-based AudioBlock ctor: (float** data, size_t numChannels, size_t numSamples)
    juce::dsp::AudioBlock<float> block(
        buffer->getArrayOfWritePointers(),
        (size_t)numChannels,
        (size_t)numSamples);

    // If your JUCE version instead expects (data, numChannels, startSample, numSamples),
    // replace the constructor above with:
    //
    // juce::dsp::AudioBlock<float> block(
    //     buffer->getArrayOfWritePointers(),
    //     (size_t) numChannels,
    //     (size_t) startSample,
    //     (size_t) numSamples);

    juce::dsp::ProcessContextReplacing<float> context(block);
    lowPassFilter.process(context);
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

    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(std::move(stream)));

    if (reader == nullptr)
        return false;

    // Prepare a new reader source in a temporary unique_ptr
    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(
        reader.release(),  // ownership of reader
        true);             // delete reader when source is deleted

    // Attach to transport BEFORE replacing our member pointer
    transportSource.stop();
    transportSource.setSource(newSource.get(),
        0,        // readAheadBufferSize
        nullptr,  // no read-ahead thread
        newSource->getAudioFormatReader()->sampleRate);

    // Now swap into the member so the old source (if any) is safely destroyed
    readerSource.reset(newSource.release());

    currentFile = audioURL.getLocalFile();
    loadedTrackName = currentFile.getFileName();
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

void DJAudioPlayer::setLowPassCutoff(double cutoffHz)
{
    lowPassCutoffHz = (float)cutoffHz;

    if (lastSampleRate > 0.0)
    {
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(
            lastSampleRate,
            juce::jlimit(20.0, lastSampleRate * 0.45, cutoffHz));

        // For JUCE versions where 'state' is private, use 'coefficients'
        lowPassFilter.coefficients = coeffs;
    }
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