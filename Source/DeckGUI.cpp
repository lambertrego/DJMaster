#include "DeckGUI.h"
#include <memory>

DeckGUI::DeckGUI(DJAudioPlayer& playerToControl,
    juce::AudioFormatManager& fm,
    const juce::String& deckDisplayName)
    : player(playerToControl), formatManager(fm), looper(playerToControl), deckName(deckDisplayName)
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
    addAndMakeVisible(filterLabel);
    addAndMakeVisible(filterSlider);

    filterLabel.setText("LPF", juce::dontSendNotification);
    filterLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
    filterLabel.setJustificationType(juce::Justification::centredLeft);

    filterSlider.addListener(this);
    filterSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    filterSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);
    filterSlider.setRange(200.0, 20000.0, 1.0);  // 200 Hz – 20 kHz
    filterSlider.setSkewFactorFromMidPoint(2000.0); // more resolution in lows
    filterSlider.setValue(20000.0);                 // default “bypass”

    deckTitle.setText(deckName, juce::dontSendNotification);
    deckTitle.setJustificationType(juce::Justification::centredLeft);
    deckTitle.setFont(juce::Font(20.0f, juce::Font::bold));
    deckTitle.setColour(juce::Label::textColourId, juce::Colours::white);

    gainLabel.setText("Gain", juce::dontSendNotification);
    speedLabel.setText("Speed", juce::dontSendNotification);
    positionLabel.setText("Position", juce::dontSendNotification);

    for (auto* label : { &gainLabel, &speedLabel, &positionLabel })
    {
        label->setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.85f));
        label->setJustificationType(juce::Justification::centredLeft);
    }

    loadButton.addListener(this);
    playButton.addListener(this);
    stopButton.addListener(this);
    loopButton.addListener(this);

    for (auto* slider : { &gainSlider, &speedSlider, &positionSlider })
    {
        slider->addListener(this);
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 72, 22);
    }

    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.8);
    speedSlider.setRange(0.25, 4.0, 0.01);
    speedSlider.setValue(1.0);
    positionSlider.setRange(0.0, 1.0, 0.001);
    thumbnail.clear();
    hasWaveform = false;
    marqueeText = "No track loaded";
    startTimerHz(30);
}

DeckGUI::~DeckGUI()
{
    stopTimer();
}

void DeckGUI::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto panel = bounds.reduced(4.0f);

    juce::ColourGradient panelGradient(juce::Colour(0xee102437), panel.getTopLeft(),
        juce::Colour(0xee0f3a44), panel.getBottomRight(), false);
    panelGradient.addColour(0.55, juce::Colour(0xdd10444c));
    g.setGradientFill(panelGradient);
    g.fillRoundedRectangle(panel, 16.0f);

    g.setColour(juce::Colours::aqua.withAlpha(0.22f));
    g.drawRoundedRectangle(panel, 16.0f, 1.6f);

    auto animatedArea = juce::Rectangle<float>(panel.getX() + 18.0f, panel.getY() + 52.0f,
        panel.getWidth() - 36.0f, 150.0f);

    // Waveform area (top half)
    auto waveformArea = animatedArea.withHeight(animatedArea.getHeight() * 0.4f).toNearestInt();
    // Platter area (bottom part)
    auto platterArea = animatedArea.withTrimmedTop(waveformArea.getHeight() + 8.0f);

    if (hasWaveform && thumbnail.getTotalLength() > 0.0)
    {
        // Background for waveform
        g.setColour(juce::Colours::black.withAlpha(0.75f));
        g.fillRoundedRectangle(waveformArea.toFloat(), 8.0f);

        // Draw waveform
        g.setColour(juce::Colours::darkcyan);
        thumbnail.drawChannels(g,
            waveformArea,
            0.0,
            thumbnail.getTotalLength(),
            1.0f);

        // Playhead cursor
        const double totalLength = thumbnail.getTotalLength();
        const double relPos = player.getPositionRelative();        // 0..1
        const double seconds = totalLength * relPos;

        if (totalLength > 0.0)
        {
            // Map time to pixel x coordinate inside waveformArea
            const float playheadX = (float)juce::jmap(seconds,
                0.0,
                totalLength,
                (double)waveformArea.getX(),
                (double)waveformArea.getRight());

            g.setColour(juce::Colours::yellow.withAlpha(0.9f));
            g.drawLine(playheadX,
                (float)waveformArea.getY(),
                playheadX,
                (float)waveformArea.getBottom(),
                2.0f);
        }
    }
    else
    {
        g.setColour(juce::Colours::black.withAlpha(0.4f));
        g.fillRoundedRectangle(waveformArea.toFloat(), 8.0f);
        g.setColour(juce::Colours::grey);
        g.drawText("No waveform",
            waveformArea,
            juce::Justification::centred);
    }

    // Animated platters below waveform
    drawAnimatedDeck(g, platterArea);

    auto marqueeArea = juce::Rectangle<int>((int)panel.getX() + 18, (int)panel.getY() + 210,
        (int)panel.getWidth() - 36, 24);
    g.setColour(juce::Colour(0xaa000000));
    g.fillRoundedRectangle(marqueeArea.toFloat(), 8.0f);
    g.setColour(juce::Colours::aqua.withAlpha(0.25f));
    g.drawRoundedRectangle(marqueeArea.toFloat(), 8.0f, 1.0f);
    drawMarqueeText(g, marqueeArea);
}

void DeckGUI::resized()
{
    auto area = getLocalBounds().reduced(18);

    deckTitle.setBounds(area.removeFromTop(26));
    area.removeFromTop(196);
    area.removeFromTop(28);

    auto buttonRow = area.removeFromTop(36);
    auto third = buttonRow.getWidth() / 3;
    loadButton.setBounds(buttonRow.removeFromLeft(third).reduced(3, 0));
    playButton.setBounds(buttonRow.removeFromLeft(third).reduced(3, 0));
    stopButton.setBounds(buttonRow.reduced(3, 0));

    area.removeFromTop(10);
    loopButton.setBounds(area.removeFromTop(28));
    area.removeFromTop(14);

    auto rowHeight = 30;

    auto gainRow = area.removeFromTop(rowHeight);
    gainLabel.setBounds(gainRow.removeFromLeft(70));
    gainSlider.setBounds(gainRow);

    area.removeFromTop(10);
    auto speedRow = area.removeFromTop(rowHeight);
    speedLabel.setBounds(speedRow.removeFromLeft(70));
    speedSlider.setBounds(speedRow);

    area.removeFromTop(10);
    auto positionRow = area.removeFromTop(rowHeight);
    positionLabel.setBounds(positionRow.removeFromLeft(70));
    positionSlider.setBounds(positionRow);

    // after positioning positionRow / positionSlider
    area.removeFromTop(10);
    auto filterRow = area.removeFromTop(rowHeight);
    filterLabel.setBounds(filterRow.removeFromLeft(70));
    filterSlider.setBounds(filterRow);
}

void DeckGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        auto chooser = std::make_shared<juce::FileChooser>("Select an audio file...");
        int chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile() && player.loadURL(juce::URL(file)))
                {
                    thumbnail.setSource(new juce::FileInputSource(file)); // NEW
                    hasWaveform = true;                                   // NEW
                   
                    marqueeText = player.getLoadedTrackName();
                    marqueeOffset = 0.0f;
                    repaint();
                }
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
    else if (slider == &filterSlider)
        player.setLowPassCutoff(slider->getValue());   // NEW
}

void DeckGUI::timerCallback()
{
    looper.update();

    if (!positionSlider.isMouseButtonDown())
        positionSlider.setValue(player.getPositionRelative(), juce::dontSendNotification);

    if (player.isPlaying())
        platterAngle += 0.06f;

    marqueeText = player.getLoadedTrackName();
    marqueeOffset += 1.0f;

    repaint();
}

void DeckGUI::drawAnimatedDeck(juce::Graphics& g, juce::Rectangle<float> area)
{
    g.setColour(juce::Colour(0x4417fff2));
    g.fillRoundedRectangle(area, 14.0f);

    auto left = area.removeFromLeft(area.getWidth() / 2.0f).reduced(8.0f);
    auto right = area.reduced(8.0f);

    auto drawPlatter = [this, &g](juce::Rectangle<float> platterArea, juce::Colour glowColour)
        {
            auto centre = platterArea.getCentre();
            auto radius = juce::jmin(platterArea.getWidth(), platterArea.getHeight()) * 0.42f;

            for (int i = 4; i >= 1; --i)
            {
                g.setColour(glowColour.withAlpha(0.06f * (float)i));
                g.fillEllipse(centre.x - radius - i * 6.0f,
                    centre.y - radius - i * 6.0f,
                    (radius * 2.0f) + i * 12.0f,
                    (radius * 2.0f) + i * 12.0f);
            }

            g.setColour(juce::Colour(0xff111111));
            g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

            g.setColour(juce::Colour(0xff2d2d2d));
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 4.0f);

            g.setColour(juce::Colour(0xff050505));
            g.fillEllipse(centre.x - radius * 0.75f, centre.y - radius * 0.75f, radius * 1.5f, radius * 1.5f);

            for (int i = 0; i < 8; ++i)
            {
                auto angle = platterAngle + juce::MathConstants<float>::twoPi * (float)i / 8.0f;
                auto x1 = centre.x + std::cos(angle) * radius * 0.18f;
                auto y1 = centre.y + std::sin(angle) * radius * 0.18f;
                auto x2 = centre.x + std::cos(angle) * radius * 0.82f;
                auto y2 = centre.y + std::sin(angle) * radius * 0.82f;
                g.setColour(glowColour.withAlpha(0.7f));
                g.drawLine(x1, y1, x2, y2, 2.2f);
            }

            g.setColour(juce::Colours::silver);
            g.fillEllipse(centre.x - radius * 0.14f, centre.y - radius * 0.14f, radius * 0.28f, radius * 0.28f);
            g.setColour(juce::Colours::black);
            g.fillEllipse(centre.x - radius * 0.05f, centre.y - radius * 0.05f, radius * 0.10f, radius * 0.10f);
        };

    drawPlatter(left, player.isPlaying() ? juce::Colours::deeppink : juce::Colours::darkslategrey);
    drawPlatter(right, player.isPlaying() ? juce::Colours::cyan : juce::Colours::darkslategrey);

    g.setColour(juce::Colours::silver.withAlpha(0.8f));
    g.drawLine(left.getRight() - 8.0f, left.getCentreY() - 28.0f,
        left.getRight() + 32.0f, left.getCentreY() + 16.0f, 3.0f);
    g.fillEllipse(left.getRight() + 25.0f, left.getCentreY() + 9.0f, 10.0f, 10.0f);
}

void DeckGUI::drawMarqueeText(juce::Graphics& g, juce::Rectangle<int> area)
{
    g.saveState();
    g.reduceClipRegion(area);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    auto textWidth = g.getCurrentFont().getStringWidthFloat(marqueeText);
    auto startX = (float)area.getX()
        - std::fmod(marqueeOffset, textWidth + 60.0f);
    auto y = (float)area.getY() + 4.0f;

    g.drawText(marqueeText,
        (int)startX,
        (int)y,
        (int)textWidth + 20,
        area.getHeight(),
        juce::Justification::centredLeft,
        false);

    g.drawText(marqueeText,
        (int)(startX + textWidth + 60.0f),
        (int)y,
        (int)textWidth + 20,
        area.getHeight(),
        juce::Justification::centredLeft,
        false);

    g.restoreState();
}