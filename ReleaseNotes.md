## DJ Master v0.3.0 – Waveforms, Filters & Playlists

This release turns DJ Master into a much more capable DJ tool: each deck now shows animated waveforms with a moving playhead, offers a simple low‑pass filter, and you can manage tracks via a playlist panel with load/save support.

### New

- **Per‑deck waveform display**
  - Each deck renders the loaded track as a waveform using JUCE’s `AudioThumbnail`.
  - A yellow playhead cursor moves across the waveform to show the current playback position.

- **Animated platter + waveform combo**
  - The neon platters now sit under a dedicated waveform strip so you get both detailed audio shape and a vibrant turntable look.

- **Low‑pass filter (LPF) per deck**
  - New LPF slider on each deck, ranging from 200 Hz to 20 kHz with a musical skew.
  - Filter is implemented with `juce::dsp::IIR::Filter`, applied in the audio path after playback and resampling.

- **Playlist panel with deck loading**
  - A playlist panel on the right lists tracks with columns for Track, Deck A, and Deck B.
  - “Load” buttons in the playlist send the selected file directly to Deck A or Deck B.
  - Supports async file selection via `FileChooser::launchAsync` so the UI stays responsive.

### Improvements

- **Safer audio file loading**
  - `DJAudioPlayer::loadURL` now uses a temporary `AudioFormatReaderSource` and only swaps it in after the transport is configured, preventing crashes when loading multiple tracks into the same deck.

- **Filter integration in audio engine**
  - The low‑pass filter is prepared with a `ProcessSpec`, shares cutoff state per deck, and processes the deck audio via `juce::dsp::ProcessContextReplacing`.

- **Playlist persistence**
  - Playlists can be saved to a simple text file (`.djplaylist`) with one full path per line.
  - Playlists can be reloaded later; missing files are skipped gracefully.

- **Waveform and title feedback**
  - Decks now expose the currently loaded file to the GUI; waveform and the scrolling track name strip stay in sync after loading from the playlist.

### Bug fixes

- Fixed a crash when loading a second track into the same deck by properly managing `AudioFormatReaderSource` ownership and stopping the transport before swapping sources.
- Fixed issues with file choosers not opening by switching playlist operations to `launchAsync` and updating to the current `getResults()` API.

### Notes

- The DSP low‑pass filter requires the JUCE `juce_dsp` module, which is now linked in the CMake target.
- Build output is still ignored by Git; Windows `.exe` binaries should be attached to GitHub Releases rather than committed to the repository.