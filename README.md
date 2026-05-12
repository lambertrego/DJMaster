# DJ Master

DJ Master is a vibrant two-deck DJ audio player built with **C++** and **JUCE** for learners who want to move beyond a plain audio demo and build something that feels like a real performance tool.

## Why this project is interesting

Most beginner JUCE projects prove that audio can play. DJ Master goes a step further: it mixes two decks, adds a crossfader, supports looping, shows the currently loaded track, and gives each deck a vivid animated platter background that rotates while music is playing.

## Features

- Two independent audio decks
- Load / Play / Stop controls per deck
- Gain, speed, and position controls
- Crossfader for blending deck A and deck B
- Loop toggle using a small template looper class
- Animated platter-style deck display that spins while playing and stops when playback stops
- Small marquee text strip that shows the loaded song name
- Clean structure for extending into waveform displays, playlists, EQ, hot cues, and beat sync

## Architecture

### Main files
- `MainComponent.*` handles the main window layout, title, crossfader, and mixer setup.
- `DJAudioPlayer.*` owns transport, file loading, playback control, gain, speed, and loaded-track state.
- `DeckGUI.*` draws each deck, handles user interaction, updates animation, and shows the scrolling song title.
- `Looper.h` provides a simple template-based loop helper using `std::atomic<bool>`.

### Flow
1. A deck loads an audio file through a JUCE file chooser.
2. `DJAudioPlayer` stores the file and exposes playback information.
3. `DeckGUI` reads player state to animate the platter and scroll the track title.
4. `MainComponent` mixes both decks and applies the crossfader balance.

## Build on Windows

1. Install Visual Studio Community with **Desktop development with C++**.
2. Download JUCE and place the `JUCE` folder next to `CMakeLists.txt`.
3. Open the project folder in Visual Studio or use the command line:

```bat
cd /d D:\DJProject\MyDjPlayer
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

4. Run the `DJMaster` target with `F5`.

## Suggested repository structure

```text
MyDjPlayer/
├── CMakeLists.txt
├── README.md
├── JUCE/
└── Source/
    ├── Main.cpp
    ├── MainComponent.h
    ├── MainComponent.cpp
    ├── DJAudioPlayer.h
    ├── DJAudioPlayer.cpp
    ├── DeckGUI.h
    ├── DeckGUI.cpp
    └── Looper.h
```

## Roadmap

- Waveform display
- Playlist panel
- Cue points and hot cues
- Beat sync and BPM analysis
- Theme switching
- MIDI controller support
- Exportable set recording

## GitHub pitch

DJ Master is a beginner-friendly but visually expressive JUCE project that turns a basic audio player into a dual-deck mixing experience. It is ideal for developers learning desktop audio, interactive GUI design, and modern C++ architecture through a project that feels alive on screen.