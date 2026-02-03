## 🎵 LoFiBox

**English** | [简体中文](README.zh.md)

**LoFiBox** is a lightweight, offline music player designed for
**ESP32-class embedded devices**.

![LoFiBox Poster](images/design/ChatGPT%20Image%202026%E5%B9%B41%E6%9C%8831%E6%97%A5%2004_36_20.png)

It focuses on doing a few things well:

* Local **MP3 / WAV playback** from SD card
* **Multi-band EQ** with real-time adjustment
* Simple, button-driven UI inspired by classic devices (iPod-style navigation)
* Predictable CPU and memory usage, suitable for long-running embedded systems

LoFiBox is not a streaming app, and it is not Hi-Fi.
It is a small player that runs entirely on the device, without networks, accounts, or background services.

---

### 🧩 Feature Highlights (with screenshots)

#### ▶️ Now Playing
![Now Playing](docs/images/nowplaying.png)
- Track info and progress at a glance: title, elapsed time, and progress bar
- Playback state and mode indicators: Sequential / Random / Repeat One
- Designed to make “what’s playing” the primary focus

#### 🎵 Library
![Library](docs/images/music.png)
- SD-card local library browsing with a clean, list-first layout
- Fast navigation and quick track switching
- Hierarchy optimized for physical buttons / rotary input

#### 📂 Playlists
![Playlists](docs/images/playlist.png)
- Playlist entry point focused on organization over complexity
- Suited to long-term, offline “set-and-play” use cases

#### 🎚️ EQ
![EQ](docs/images/eq.png)
- Multi-band EQ with real-time tuning
- Lightweight UI for quick, ear-first adjustments

#### ⚙️ Settings
![Settings](docs/images/setting.png)
- Centralized controls for brightness, sleep, and behavior
- Shallow hierarchy, low-friction flow for small screens and physical keys

### ✨ Design Goals

* **Offline-first**
  No Bluetooth, no Wi-Fi, no dependencies on phones or cloud services.

* **Embedded-friendly**
  Designed for ESP32-class MCUs with limited RAM, flash, and display bandwidth.

* **Playable, not flashy**
  The UI favors clarity and tactile interaction over animations or visual effects.

* **EQ as a first-class feature**
  Multi-band equalizer implemented in software, suitable for low-power audio pipelines.

---

### 🎛 Interface Philosophy

The UI takes inspiration from **Apple’s classic iPod**:

* List-based navigation
* Clear hierarchy: Library → Track → Now Playing → EQ
* Minimal screen updates
* Button-centric interaction instead of touch gestures

The goal is to make music playback feel **deliberate and calm**, even on small embedded screens.

---

### 🔧 Technical Overview

* Target platform: **ESP32 / ESP32-S3**
* Audio formats: **MP3, WAV**
* Audio pipeline:
  SD → Decoder → PCM → Multi-band EQ → Output
* UI framework: **LVGL**
* Output: I2S DAC or onboard audio path (device-dependent)

---

### ⌨️ Key Controls

#### Playback Control
- **A** - Play/Pause toggle
- **N** - Next song
- **P** - Previous song
- **V** - Cycle volume levels (step 1, range 0-21)

#### Playback Mode
- **M** - Toggle playback mode
  - SEQ: Sequential playback
  - RND: Random playback
  - ONE: Single repeat

#### Screen Control
- **L** - Cycle screen brightness (5 levels)
- **S** - Screen off/on toggle (saves brightness when off, restores when on)

#### File Management
- **D** - Show delete confirmation dialog
  - **Y** - Confirm delete currently selected song
  - **C** - Cancel delete

#### Screenshot
- **F** - Capture screenshot and save to SD card
  - Screenshots are saved in `/screen` directory
  - Filename format: `screenshot_YYYYMMDD_HHMMSS.bmp`
  - Automatically creates `/screen` directory if it doesn't exist

---

### 📌 What LoFiBox Is (and Isn’t)

**LoFiBox is:**

* A self-contained embedded music player
* Suitable for DIY devices, pagers, handhelds, and experimental hardware
* Easy to integrate as a subsystem in larger projects

**LoFiBox is not:**

* A smartphone music app
* A streaming or cloud-connected player
* A Hi-Fi audio reference design
