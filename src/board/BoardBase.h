#pragma once

#include <Arduino.h>

class BoardBase {
public:
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
};

#ifndef DEVICE_MAX_BRIGHTNESS_LEVEL
#define DEVICE_MAX_BRIGHTNESS_LEVEL 16
#endif
#ifndef DEVICE_MIN_BRIGHTNESS_LEVEL
#define DEVICE_MIN_BRIGHTNESS_LEVEL 0
#endif

extern BoardBase &board;
