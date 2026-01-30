#pragma once

#include "BoardBase.h"
#include "display/DisplayInterface.h"
#include "display/drivers/ST7796.h"
#include <ExtensionIOXL9555.hpp>
#include "pins_arduino.h"

class TLoraPagerBoard : public BoardBase
{
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
    bool readKey(uint32_t* key) override;
    uint16_t displayWidth() const override;
    uint16_t displayHeight() const override;
    void displayPushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* colors) override;
    bool isSDReady() const override;
    bool initAudio(uint8_t& bclk, uint8_t& lrck, uint8_t& dout, int8_t& mclk) override;

  private:
    uint8_t brightness_ = DEVICE_MAX_BRIGHTNESS_LEVEL;
    uint8_t keyboard_brightness_ = 0;
    bool keyboard_ready_ = false;
    bool sd_ready_ = false;
    display::drivers::ST7796 st7796_;
    LilyGoDispArduinoSPI display_ = LilyGoDispArduinoSPI(
        DISP_WIDTH,
        DISP_HEIGHT,
        display::drivers::ST7796::getInitCommands(),
        display::drivers::ST7796::getInitCommandsCount(),
        display::drivers::ST7796::getRotationConfig(DISP_WIDTH, DISP_HEIGHT, 49, 49));
    ExtensionIOXL9555 io_;
    bool display_ready_ = false;
};
