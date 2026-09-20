#include "M5Core2AudioFacesBoard.h"

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)

#include <SD.h>
#include <SPI.h>
#include <driver/rmt.h>
#include <lvgl.h>

#include "app/input_keys.h"

namespace {
constexpr uint32_t kSdFrequency = 25000000;
constexpr uint32_t kRepeatDelayMs = 350;
constexpr uint32_t kRepeatIntervalMs = 80;
// M5 Module Audio's STM32 controller exposes the TRRS jack state on this
// read-only register: 0 = open, 1 = headphone/headset inserted.
constexpr uint8_t kModuleAudioControllerAddress = 0x33;
constexpr uint8_t kModuleAudioHeadphoneStatusRegister = 0x20;
constexpr uint32_t kMusicLightUpdateMs = 40;
// The Bottom3 schematic shows LEDnet feeding two parallel SK6812 strings:
// LED1..LED5 on one side and LED6..LED10 on the other. One five-pixel frame
// therefore updates both sides in lockstep; it is not a ten-pixel daisy chain.
constexpr uint8_t kBottom3PixelsPerSide = 5;
constexpr uint8_t kBottom3LedPinCore2 = 25;

struct Bottom3Pixel {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// Bottom3's side light bars are not part of the Gamepad3 I2C controller.
// They are ten daisy-chained SK6812 pixels and the physical selector on the
// base routes its DIN to GPIO25 when set for Core2. GPIO15 is used by legacy
// hosts but can be shared by Core2 peripherals, so this backend must never
// drive it.
void writeBottom3Pixels(const Bottom3Pixel *pixels) {
  constexpr rmt_channel_t kChannel = RMT_CHANNEL_0;
  // Bottom3 uses three-channel SK6812 LEDs. Its RGB bars are physically
  // parallel, so send RGB/GRB data for one five-pixel side only.
  constexpr size_t kBitsPerPixel = 24;
  constexpr size_t kItemCount = kBottom3PixelsPerSide * kBitsPerPixel;
  static bool initialized = false;
  static rmt_item32_t items[kItemCount];

  if (!initialized) {
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(
        static_cast<gpio_num_t>(kBottom3LedPinCore2), kChannel);
    config.clk_div = 2; // 40 MHz: one tick is 25 ns.
    config.tx_config.loop_en = false;
    rmt_config(&config);
    rmt_driver_install(kChannel, 0, 0);
    initialized = true;
  }

  // SK6812 expects GRB. At a 40 MHz RMT clock: 0 = 0.35/0.90 us and
  // 1 = 0.70/0.55 us; the 80 us idle gap after rmt_write_items latches it.
  size_t item = 0;
  for (uint8_t pixel = 0; pixel < kBottom3PixelsPerSide; ++pixel) {
    const uint8_t bytes[] = {pixels[pixel].green, pixels[pixel].red,
                             pixels[pixel].blue};
    for (uint8_t byte : bytes) {
      for (int bit = 7; bit >= 0; --bit) {
        const bool one = (byte & (1U << bit)) != 0;
        items[item].level0 = 1;
        items[item].duration0 = one ? 28 : 14;
        items[item].level1 = 0;
        items[item].duration1 = one ? 22 : 36;
        ++item;
      }
    }
  }
  rmt_write_items(kChannel, items, kItemCount, true);
}

void turnBottom3Off() {
  const Bottom3Pixel off[kBottom3PixelsPerSide] = {};
  writeBottom3Pixels(off);
}

uint8_t scaleColor(uint8_t color, uint8_t amount) {
  return static_cast<uint8_t>((static_cast<uint16_t>(color) * amount) / 255U);
}

bool isRepeatable(gamepad3_btn_t button) {
  return button == GAMEPAD3_BTN_UP || button == GAMEPAD3_BTN_DOWN ||
         button == GAMEPAD3_BTN_LEFT || button == GAMEPAD3_BTN_RIGHT;
}
} // namespace

uint32_t M5Core2AudioFacesBoard::begin(uint32_t disable_hw_init) {
  (void)disable_hw_init;

  auto config = M5.config();
  // The player owns I2S directly and switches between the Module Audio pins
  // and Core2's speaker pins at runtime. Do not create M5Unified's separate
  // speaker/mic I2S drivers, which would compete for those same peripherals.
  config.internal_mic = false;
  config.internal_spk = false;
  config.internal_imu = false;
  config.internal_rtc = false;
  config.led_brightness = 0;
  M5.begin(config);
  M5.Display.setRotation(1);
  // LVGL emits native-endian RGB565 pixels. M5GFX otherwise treats the input
  // passed to pushImage() as byte-swapped RGB565, which corrupts the LCD
  // colors while geometry and input still appear correct.
  M5.Display.setSwapBytes(true);
  setBrightness(brightness_);
  Serial.printf("[CORE2] LCD %ux%u brightness=%u\n", M5.Display.width(),
                M5.Display.height(), brightness_);

  const int sck = M5.getPin(m5::pin_name_t::sd_spi_sclk);
  const int miso = M5.getPin(m5::pin_name_t::sd_spi_miso);
  const int mosi = M5.getPin(m5::pin_name_t::sd_spi_mosi);
  const int cs = M5.getPin(m5::pin_name_t::sd_spi_cs);
  SPI.begin(sck, miso, mosi, cs);
  sd_ready_ = SD.begin(cs, SPI, kSdFrequency);

  gamepad_ready_ = gamepad_.begin(&M5.In_I2C, M5FACES_BOTTOM3_ADDR,
                                  M5FACES_I2C_FREQ_STANDARD) == M5FACES_OK;
  if (gamepad_ready_) {
    // The first read can contain stale controller response data.
    gamepad_.update();
  }
  turnBottom3Off();
  bottom3_leds_active_ = false;
  return 0;
}

void M5Core2AudioFacesBoard::wakeUp() { setBrightness(brightness_); }

void M5Core2AudioFacesBoard::handlePowerButton() {
  M5.update();
  if (M5.BtnPWR.wasHold()) {
    softwareShutdown();
  }
}

void M5Core2AudioFacesBoard::softwareShutdown() {
  M5.Display.setBrightness(0);
  M5.Power.powerOff();
}

void M5Core2AudioFacesBoard::setBrightness(uint8_t level) {
  if (level > DEVICE_MAX_BRIGHTNESS_LEVEL) {
    level = DEVICE_MAX_BRIGHTNESS_LEVEL;
  }
  brightness_ = level;
  const uint16_t scaled =
      (static_cast<uint16_t>(level) * 255) / DEVICE_MAX_BRIGHTNESS_LEVEL;
  M5.Display.setBrightness(static_cast<uint8_t>(scaled));
}

uint8_t M5Core2AudioFacesBoard::getBrightness() const { return brightness_; }

bool M5Core2AudioFacesBoard::hasKeyboard() const { return false; }

void M5Core2AudioFacesBoard::keyboardSetBrightness(uint8_t level) {
  (void)level;
}

uint8_t M5Core2AudioFacesBoard::keyboardGetBrightness() const { return 0; }

bool M5Core2AudioFacesBoard::isRTCReady() const { return false; }

bool M5Core2AudioFacesBoard::isCharging() const {
  return M5.Power.isCharging() == m5::Power_Class::is_charging;
}

int M5Core2AudioFacesBoard::getBatteryLevel() const {
  // Core2's PMIC exposes the actual charge estimate.  Faces shares the M-Bus
  // but does not replace that PMIC reading, so returning -1 here previously
  // made the topbar permanently show an empty placeholder icon.
  const int level = M5.Power.getBatteryLevel();
  return (level >= 0 && level <= 100) ? level : -1;
}

bool M5Core2AudioFacesBoard::mapGamepadKey(gamepad3_btn_t button,
                                           uint32_t &key) {
  switch (button) {
  case GAMEPAD3_BTN_UP:
    key = LV_KEY_UP;
    return true;
  case GAMEPAD3_BTN_DOWN:
    key = LV_KEY_DOWN;
    return true;
  case GAMEPAD3_BTN_LEFT:
    key = LV_KEY_LEFT;
    return true;
  case GAMEPAD3_BTN_RIGHT:
    key = LV_KEY_RIGHT;
    return true;
  case GAMEPAD3_BTN_A:
    key = LV_KEY_ENTER;
    return true;
  case GAMEPAD3_BTN_B:
    key = LV_KEY_ESC;
    return true;
  case GAMEPAD3_BTN_START:
    key = APP_KEY_PLAY_PAUSE;
    return true;
  case GAMEPAD3_BTN_SELECT:
    key = APP_KEY_MODE_CYCLE;
    return true;
  default:
    return false;
  }
}

bool M5Core2AudioFacesBoard::readKey(uint32_t *key) {
  if (!key || !gamepad_ready_) {
    return false;
  }

  // LVGL may ask both its keyboard and pointer drivers for input several
  // times in one render pass.  M5.update() already runs once at the top of
  // the main loop; limiting this Faces I2C transaction keeps a button-driven
  // screen refresh from starving the single-threaded audio decoder.
  const uint32_t now = millis();
  if (now - last_gamepad_poll_ms_ < 15) {
    return false;
  }
  last_gamepad_poll_ms_ = now;
  gamepad_.update();
  constexpr gamepad3_btn_t kButtons[] = {
      GAMEPAD3_BTN_UP,    GAMEPAD3_BTN_DOWN,  GAMEPAD3_BTN_LEFT,
      GAMEPAD3_BTN_RIGHT, GAMEPAD3_BTN_A,     GAMEPAD3_BTN_B,
      GAMEPAD3_BTN_START, GAMEPAD3_BTN_SELECT};

  for (const auto button : kButtons) {
    if (!gamepad_.isButtonJustPressed(button) || !mapGamepadKey(button, *key)) {
      continue;
    }
    if (isRepeatable(button)) {
      repeat_button_ = button;
      repeat_key_ = *key;
      repeat_after_ms_ = millis() + kRepeatDelayMs;
    }
    return true;
  }

  if (repeat_key_ == 0 || !gamepad_.isButtonPressed(repeat_button_)) {
    repeat_key_ = 0;
    return false;
  }
  if (static_cast<int32_t>(now - repeat_after_ms_) < 0) {
    return false;
  }
  *key = repeat_key_;
  repeat_after_ms_ = now + kRepeatIntervalMs;
  return true;
}

bool M5Core2AudioFacesBoard::readTouch(TouchState *state) {
  if (!state) {
    return false;
  }

  // M5Unified rotates the touch coordinates along with M5.Display, so these
  // values are already in the 320x240 landscape coordinate system LVGL uses.
  *state = {};
  if (M5.Touch.getCount() == 0) {
    touch_pressed_ = false;
    return true;
  }

  const auto touch = M5.Touch.getDetail(0);
  const int max_x = static_cast<int>(M5.Display.width()) - 1;
  const int max_y = static_cast<int>(M5.Display.height()) - 1;
  state->x =
      static_cast<uint16_t>(constrain(static_cast<int>(touch.x), 0, max_x));
  state->y =
      static_cast<uint16_t>(constrain(static_cast<int>(touch.y), 0, max_y));
  state->pressed = touch.isPressed();
  if (!state->pressed) {
    touch_pressed_ = false;
    return true;
  }
  if (!touch_pressed_) {
    Serial.printf("[CORE2 TOUCH] down x=%u y=%u\n",
                  static_cast<unsigned>(state->x),
                  static_cast<unsigned>(state->y));
  }
  touch_pressed_ = true;
  return true;
}

uint16_t M5Core2AudioFacesBoard::displayWidth() const {
  return M5.Display.width();
}

uint16_t M5Core2AudioFacesBoard::displayHeight() const {
  return M5.Display.height();
}

void M5Core2AudioFacesBoard::displayPushColors(uint16_t x, uint16_t y,
                                               uint16_t w, uint16_t h,
                                               const uint16_t *colors) {
  M5.Display.pushImage(x, y, w, h, colors);
}

bool M5Core2AudioFacesBoard::isSDReady() const { return sd_ready_; }

bool M5Core2AudioFacesBoard::initAudio(uint8_t &bclk, uint8_t &lrck,
                                       uint8_t &dout, int8_t &mclk) {
  bool ok = codec_.init();
  ok &= codec_.setDACOutput(DAC_OUTPUT_ALL);
  ok &= codec_.setBitsSample(ES_MODULE_DAC, BIT_LENGTH_16BITS);
  // Keep the analog headphone stage at unity gain. User volume is then
  // controlled digitally by the player (0..21) instead of starting from an
  // artificially quiet DAC setting.
  ok &= codec_.setDACVolume(100);
  // LoFiBox is playback-only. The upstream ES8388 helper configures both
  // record and playback by default, which needlessly keeps the mic bias and
  // ADC path powered. Disable the entire ADC section after its generic init.
  ok &= M5.In_I2C.writeRegister8(ES8388_ADDR, ES8388_ADCPOWER, 0xFF,
                                 M5FACES_I2C_FREQ_STANDARD);
  codec_ready_ = ok;
  // ES8388::init enables the output path. Mark it active so the following
  // idle transition actually gates that path before any track starts.
  audio_active_ = ok;

  // Start from the low-power Core2 speaker route. player_init() resolves the
  // saved Auto/Speaker/Headphones preference immediately afterwards.
  audio_output_ = AudioOutput::Speaker;
  getAudioOutputPinout(audio_output_, bclk, lrck, dout, mclk);
  setAudioSampleRate(44100);
  setAudioActive(false);
  return ok;
}

bool M5Core2AudioFacesBoard::headphonesInserted() {
  uint8_t inserted = 0;
  if (!M5.In_I2C.readRegister(kModuleAudioControllerAddress,
                              kModuleAudioHeadphoneStatusRegister, &inserted, 1,
                              M5FACES_I2C_FREQ_STANDARD)) {
    return false;
  }
  return inserted == 1;
}

bool M5Core2AudioFacesBoard::getAudioOutputPinout(AudioOutput output,
                                                  uint8_t &bclk, uint8_t &lrck,
                                                  uint8_t &dout, int8_t &mclk) {
  if (output == AudioOutput::Headphones) {
    bclk = M5.getPin(m5::pin_name_t::mbus_pin22);
    lrck = M5.getPin(m5::pin_name_t::mbus_pin21);
    dout = M5.getPin(m5::pin_name_t::mbus_pin23);
    mclk = M5.getPin(m5::pin_name_t::mbus_pin24);
    return true;
  }

  // Core2's NS4168 amplifier is connected to this independent I2S bus.
  // It has no MCLK input.
  bclk = 12;
  lrck = 0;
  dout = 2;
  mclk = -1;
  return true;
}

void M5Core2AudioFacesBoard::setCore2SpeakerPower(bool enabled) {
  // This is the same PMIC gate used by M5Unified's Core2 speaker callback,
  // exposed directly because LoFiBox owns the I2S stream rather than using
  // M5Unified's separate Speaker instance.
  switch (M5.Power.getType()) {
  case m5::Power_Class::pmic_axp192:
    M5.Power.Axp192.setGPIO2(enabled);
    break;
  case m5::Power_Class::pmic_axp2101:
    M5.Power.Axp2101.setALDO3(enabled ? 3300 : 0);
    break;
  default:
    break;
  }
}

void M5Core2AudioFacesBoard::updateExternalCodecOutput() {
  if (!codec_ready_) {
    return;
  }
  const bool headphones_active =
      audio_active_ && audio_output_ == AudioOutput::Headphones;
  codec_.setDACmute(!headphones_active);
  codec_.setDACOutput(headphones_active ? DAC_OUTPUT_ALL
                                        : static_cast<es_dac_output_t>(0));
}

void M5Core2AudioFacesBoard::setAudioOutput(AudioOutput output) {
  if (audio_output_ == output) {
    return;
  }
  audio_output_ = output;
  setCore2SpeakerPower(audio_active_ && output == AudioOutput::Speaker);
  updateExternalCodecOutput();
  Serial.printf("[CORE2 AUDIO] route=%s active=%d\n",
                output == AudioOutput::Headphones ? "headphones" : "speaker",
                audio_active_ ? 1 : 0);
}

bool M5Core2AudioFacesBoard::mapSampleRate(uint32_t sample_rate,
                                           es_sample_rate_t &rate) {
  switch (sample_rate) {
  case 8000:
    rate = SAMPLE_RATE_8K;
    return true;
  case 11025:
    rate = SAMPLE_RATE_11K;
    return true;
  case 16000:
    rate = SAMPLE_RATE_16K;
    return true;
  case 24000:
    rate = SAMPLE_RATE_24K;
    return true;
  case 32000:
    rate = SAMPLE_RATE_32K;
    return true;
  case 44100:
    rate = SAMPLE_RATE_44K;
    return true;
  case 48000:
    rate = SAMPLE_RATE_48K;
    return true;
  default:
    return false;
  }
}

void M5Core2AudioFacesBoard::setAudioSampleRate(uint32_t sample_rate) {
  if (!codec_ready_) {
    return;
  }
  es_sample_rate_t rate = SAMPLE_RATE_44K;
  if (mapSampleRate(sample_rate, rate)) {
    codec_.setSampleRate(rate);
  }
}

void M5Core2AudioFacesBoard::setAudioActive(bool active) {
  if (!codec_ready_ || audio_active_ == active) {
    return;
  }
  audio_active_ = active;
  setCore2SpeakerPower(active && audio_output_ == AudioOutput::Speaker);
  // The Audio Module remains powered by the M-Bus because it shares that bus
  // with Faces, but its analog DAC/output stage is powered only while its
  // headphone route is actively playing.
  updateExternalCodecOutput();
}

void M5Core2AudioFacesBoard::updateMusicLights(uint8_t level, bool playing,
                                               bool enabled) {
  const bool active = enabled && playing;
  const uint32_t now = millis();
  static uint32_t last_diagnostic_ms = 0;
  if (now - last_diagnostic_ms >= 1000) {
    last_diagnostic_ms = now;
    Serial.printf("[CORE2 LIGHT] enabled=%d playing=%d level=%u\n",
                  enabled ? 1 : 0, playing ? 1 : 0,
                  static_cast<unsigned>(level));
  }
  if (active && now - last_music_light_update_ms_ < kMusicLightUpdateMs) {
    return;
  }
  last_music_light_update_ms_ = now;

  if (!active) {
    if (bottom3_leds_active_) {
      turnBottom3Off();
      bottom3_leds_active_ = false;
    }
    return;
  }

  // Bottom3 has two parallel five-pixel RGB bars. A blue/purple comet moves
  // across both sides at once; music level controls its brightness and trail.
  // The intentionally modest cap avoids the base's high-current full-white
  // mode.
  Bottom3Pixel pixels[kBottom3PixelsPerSide] = {};
  const uint8_t position = static_cast<uint8_t>((now / 55U) % 9U);
  const uint8_t head = position < kBottom3PixelsPerSide
                           ? position
                           : static_cast<uint8_t>(8U - position);
  const uint8_t base =
      static_cast<uint8_t>(18U + (static_cast<uint16_t>(level) * 72U) / 100U);
  for (uint8_t pixel = 0; pixel < kBottom3PixelsPerSide; ++pixel) {
    const uint8_t distance = pixel > head ? pixel - head : head - pixel;
    const uint8_t trail =
        distance >= 5 ? 0 : static_cast<uint8_t>((5U - distance) * 51U);
    // Add a faint blue underglow so the physical two-side layout remains
    // legible between beats, then layer a cyan/magenta travelling highlight.
    const uint8_t intensity = static_cast<uint8_t>(
        (static_cast<uint16_t>(base) * (35U + trail)) / 255U);
    pixels[pixel] = {
        scaleColor(static_cast<uint8_t>(70U + (pixel & 1U) * 55U), intensity),
        scaleColor(static_cast<uint8_t>(20U + trail / 3U), intensity),
        scaleColor(255U, intensity)};
  }
  writeBottom3Pixels(pixels);
  bottom3_leds_active_ = true;
}

#endif // defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
