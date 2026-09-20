#pragma once

#include "BoardBase.h"

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)

#include <M5Faces.h>
#include <M5Unified.h>
#include <es8388.hpp>

class M5Core2AudioFacesBoard : public BoardBase {
public:
  uint32_t begin(uint32_t disable_hw_init = 0) override;
  void wakeUp() override;
  void handlePowerButton() override;
  void softwareShutdown() override;

  void setBrightness(uint8_t level) override;
  uint8_t getBrightness() const override;

  bool hasKeyboard() const override;
  void keyboardSetBrightness(uint8_t level) override;
  uint8_t keyboardGetBrightness() const override;

  bool isRTCReady() const override;
  bool isCharging() const override;
  int getBatteryLevel() const override;

  bool readKey(uint32_t *key) override;
  bool readTouch(TouchState *state) override;

  uint16_t displayWidth() const override;
  uint16_t displayHeight() const override;
  void displayPushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         const uint16_t *colors) override;

  bool isSDReady() const override;
  bool initAudio(uint8_t &bclk, uint8_t &lrck, uint8_t &dout,
                 int8_t &mclk) override;
  bool supportsAudioOutputSelection() const override { return true; }
  bool headphonesInserted() override;
  bool getAudioOutputPinout(AudioOutput output, uint8_t &bclk,
                            uint8_t &lrck, uint8_t &dout,
                            int8_t &mclk) override;
  void setAudioOutput(AudioOutput output) override;
  void setAudioSampleRate(uint32_t sample_rate) override;
  void setAudioActive(bool active) override;
  bool supportsMusicLights() const override { return true; }
  void updateMusicLights(uint8_t level, bool playing, bool enabled) override;

private:
  static bool mapSampleRate(uint32_t sample_rate, es_sample_rate_t &rate);
  static bool mapGamepadKey(gamepad3_btn_t button, uint32_t &key);
  void setCore2SpeakerPower(bool enabled);
  void updateExternalCodecOutput();

  uint8_t brightness_ = DEVICE_MAX_BRIGHTNESS_LEVEL;
  bool sd_ready_ = false;
  bool gamepad_ready_ = false;
  bool codec_ready_ = false;
  bool audio_active_ = false;
  AudioOutput audio_output_ = AudioOutput::Speaker;
  bool touch_pressed_ = false;
  // Bottom3 is a separate SK6812 chain, not the Gamepad3 controller's LEDs.
  // Track its state so turning lights off sends one black frame instead of
  // continuously occupying RMT and the CPU while the player is idle.
  bool bottom3_leds_active_ = false;
  uint32_t last_gamepad_poll_ms_ = 0;
  uint32_t last_music_light_update_ms_ = 0;
  M5Faces_Gamepad3 gamepad_;
  ES8388 codec_{&M5.In_I2C};
  gamepad3_btn_t repeat_button_ = GAMEPAD3_BTN_UP;
  uint32_t repeat_key_ = 0;
  uint32_t repeat_after_ms_ = 0;
};

#endif // defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
