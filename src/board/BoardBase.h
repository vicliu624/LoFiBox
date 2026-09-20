#pragma once

#include <Arduino.h>

class BoardBase {
public:
  // A board can expose more than one physical playback path. The player
  // selects the route, then asks the board for the matching I2S pinout.
  enum class AudioOutput {
    Speaker = 0,
    Headphones,
  };

  struct TouchState {
    uint16_t x = 0;
    uint16_t y = 0;
    bool pressed = false;
  };

  virtual ~BoardBase() = default;

  virtual uint32_t begin(uint32_t disable_hw_init = 0) = 0;
  virtual void wakeUp() = 0;
  virtual void handlePowerButton() = 0;
  virtual void softwareShutdown() = 0;

  virtual void setBrightness(uint8_t level) = 0;
  virtual uint8_t getBrightness() const = 0;

  virtual bool hasKeyboard() const = 0;
  virtual void keyboardSetBrightness(uint8_t level) = 0;
  virtual uint8_t keyboardGetBrightness() const = 0;

  virtual bool isRTCReady() const = 0;
  virtual bool isCharging() const = 0;
  virtual int getBatteryLevel() const = 0;

  // Input
  virtual bool readKey(uint32_t *key) = 0;
  // Returns whether this board has a touch controller.  Boards without one
  // retain the default false result; supported boards always fill `state`,
  // including a released sample when no finger is down.
  virtual bool readTouch(TouchState *state) {
    if (state) {
      *state = {};
    }
    return false;
  }

  // Display
  virtual uint16_t displayWidth() const = 0;
  virtual uint16_t displayHeight() const = 0;
  virtual void displayPushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                                 const uint16_t *colors) = 0;

  // Storage
  virtual bool isSDReady() const = 0;

  // Audio
  virtual bool initAudio(uint8_t &bclk, uint8_t &lrck, uint8_t &dout,
                         int8_t &mclk) = 0;

  // Boards with a local speaker plus a separate headphone codec can opt in
  // to runtime routing. The defaults retain the single-output behaviour of
  // Pager and the other existing targets.
  virtual bool supportsAudioOutputSelection() const { return false; }
  virtual bool headphonesInserted() { return false; }
  virtual bool getAudioOutputPinout(AudioOutput output, uint8_t &bclk,
                                    uint8_t &lrck, uint8_t &dout,
                                    int8_t &mclk) {
    (void)output;
    (void)bclk;
    (void)lrck;
    (void)dout;
    (void)mclk;
    return false;
  }
  virtual void setAudioOutput(AudioOutput output) { (void)output; }

  // Called by the player when its decoder changes the I2S sample rate. Boards
  // without a separately clocked codec can retain the default no-op behavior.
  virtual void setAudioSampleRate(uint32_t sample_rate) { (void)sample_rate; }

  // Called when the player begins or stops supplying PCM.  A hardware codec
  // can mute/power-gate its output path while the player is idle.
  virtual void setAudioActive(bool active) { (void)active; }

  // Optional visualizers. The board receives the already-smoothed PCM level
  // from the foreground loop, so LED I2C traffic never runs in the decoder.
  virtual bool supportsMusicLights() const { return false; }
  virtual void updateMusicLights(uint8_t level, bool playing, bool enabled) {
    (void)level;
    (void)playing;
    (void)enabled;
  }
};

#ifndef DEVICE_MAX_BRIGHTNESS_LEVEL
#define DEVICE_MAX_BRIGHTNESS_LEVEL 16
#endif
#ifndef DEVICE_MIN_BRIGHTNESS_LEVEL
#define DEVICE_MIN_BRIGHTNESS_LEVEL 0
#endif

extern BoardBase &board;
