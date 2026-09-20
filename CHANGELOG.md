# Changelog

All notable changes to this project will be documented in this file.

## [0.2.0-beta.1] - 2026-09-20

### Added
- M5Stack Core2 v1.3 backend for the Core2 + Audio Module M144 + Faces
  Bottom3 + Gamepad3 stack.
- Core2 320x240 layouts, touch input, Gamepad3 navigation, SD-card support,
  Core2 speaker output, and Module Audio headphone output selection.
- ES8388 sample-rate synchronization for 8–48 kHz playback and a dedicated
  decoder task to keep audio stable during display and input work.
- Wi-Fi setup UI with on-device SSID/password entry, on-demand lyric download,
  local lyric cache, and real-time scrolling lyrics on Pager and Core2.
- Optional Bottom3 music-light animation with a persistent setting. It drives
  only the Bottom3's two parallel five-pixel RGB bars.
- CI builds for T-LoRa Pager, Cardputer, and Core2 Audio Faces.

### Changed
- Pager and Core2 now share responsive screen layouts, including the compact
  Core2 Now Playing and EQ screens.
- Wi-Fi is disabled by default and stays powered down until explicitly enabled
  in Settings.
- Core2 audio and visual peripherals enter their idle states when playback is
  inactive to reduce unnecessary heat and background power use.

### Hardware notes
- For Core2, set the Bottom3 physical LED IO selector to `1 / G25`; the
  firmware cannot change this hardware routing.
- This is a beta release. Run the on-device acceptance checklist in
  `docs/hardware/m5stack-core2-audio-faces.md` before relying on it for a
  long unattended session.

## [0.1.0-alpha] - 2026-02-03

### Added
- Offline local playback for MP3/WAV from SD card
- Library scanning and list-based browsing
- Now Playing screen with track metadata, progress, and playback modes
- Multi-band EQ screen with real-time adjustment
- Playlist browsing screens
- Settings for brightness, screen timeout, and sleep behavior
- Screenshot capture to `/screen` as BMP using LVGL snapshot
- Physical key navigation for playback, mode, brightness, and file actions
- LVGL-based UI optimized for ESP32-class devices
