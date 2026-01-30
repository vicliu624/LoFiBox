## 🎵 LoFiBox

**LoFiBox** is a lightweight, offline music player designed for
**ESP32-class embedded devices**.

It focuses on doing a few things well:

* Local **MP3 / WAV playback** from SD card
* **Multi-band EQ** with real-time adjustment
* Simple, button-driven UI inspired by classic devices (iPod-style navigation)
* Predictable CPU and memory usage, suitable for long-running embedded systems

LoFiBox is not a streaming app, and it is not Hi-Fi.
It is a small player that runs entirely on the device, without networks, accounts, or background services.

---

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

### 📌 What LoFiBox Is (and Isn’t)

**LoFiBox is:**

* A self-contained embedded music player
* Suitable for DIY devices, pagers, handhelds, and experimental hardware
* Easy to integrate as a subsystem in larger projects

**LoFiBox is not:**

* A smartphone music app
* A streaming or cloud-connected player
* A Hi-Fi audio reference design
