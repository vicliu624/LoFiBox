#include "CardputerBoard.h"

#if defined(BOARD_CARDPUTER_ADV)

#include <cctype>

#include <M5Cardputer.h>
#include <lvgl.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

namespace
{
constexpr int kSdSck = 40;
constexpr int kSdMiso = 39;
constexpr int kSdMosi = 14;
constexpr int kSdCs = 12;

constexpr int kI2cSda = 8;
constexpr int kI2cScl = 9;
constexpr uint8_t kEs8311Addr = 0x18;
constexpr uint8_t kAw88298Addr = 0x36;
constexpr uint8_t kAw9523Addr = 0x58;
constexpr uint8_t kTca8418Addr = 0x34;

constexpr int kI2SBclk = 41;
constexpr int kI2SLrck = 43;
constexpr int kI2SDout = 42;
constexpr int kAmpEnablePin = 46;
constexpr int kHpDetectPin = 17;

bool i2c_write_reg(uint8_t addr, uint8_t reg, uint8_t val)
{
    uint8_t data = val;
    return M5.In_I2C.writeRegister(addr, reg, &data, 1, 400000);
}

bool init_es8311()
{
    Wire.end();
    delay(20);
    Wire.begin(kI2cSda, kI2cScl, 100000);
    Wire.setTimeOut(50);
    delay(5);

    struct RegVal { uint8_t reg; uint8_t val; };
    static constexpr RegVal kInitSeq[] = {
        {0x00, 0x80},
        {0x01, 0xB5},
        {0x02, 0x18},
        {0x0D, 0x01},
        {0x12, 0x00},
        {0x13, 0x10},
        {0x32, 0xBF},
        {0x37, 0x08},
    };

    bool ok = true;
    for (const auto& entry : kInitSeq) {
        if (!i2c_write_reg(kEs8311Addr, entry.reg, entry.val)) {
            ok = false;
        }
    }

    if (kAmpEnablePin >= 0) {
        pinMode(kAmpEnablePin, OUTPUT);
        digitalWrite(kAmpEnablePin, HIGH);
    }
    if (kHpDetectPin >= 0) {
        pinMode(kHpDetectPin, INPUT_PULLUP);
    }
    return ok;
}

bool init_aw88298()
{
    Wire.end();
    delay(20);
    Wire.begin(kI2cSda, kI2cScl, 100000);
    Wire.setTimeOut(50);
    delay(5);

    auto writeAw = [](uint8_t reg, uint16_t value) {
        uint8_t payload[2] = {static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF)};
        return M5.In_I2C.writeRegister(kAw88298Addr, reg, payload, 2, 400000);
    };

    bool ok = true;
    ok &= writeAw(0x61, 0x0673);
    ok &= writeAw(0x04, 0x4040);
    ok &= writeAw(0x05, 0x0008);
    ok &= writeAw(0x06, 0x14C0);
    ok &= writeAw(0x0C, 0x0064);
    return ok;
}
} // namespace

uint32_t CardputerBoard::begin(uint32_t disable_hw_init)
{
    (void)disable_hw_init;
    M5Cardputer.begin();
    M5Cardputer.Display.setRotation(1);
    keyboard_ready_ = true;
    SPI.begin(kSdSck, kSdMiso, kSdMosi);
    sd_ready_ = SD.begin(kSdCs);
    return 0;
}

void CardputerBoard::wakeUp() {}

void CardputerBoard::handlePowerButton() {}

void CardputerBoard::softwareShutdown() {}

void CardputerBoard::setBrightness(uint8_t level)
{
    brightness_ = level;
}

uint8_t CardputerBoard::getBrightness() const
{
    return brightness_;
}

bool CardputerBoard::hasKeyboard() const
{
    return keyboard_ready_;
}

void CardputerBoard::keyboardSetBrightness(uint8_t level)
{
    keyboard_brightness_ = level;
}

uint8_t CardputerBoard::keyboardGetBrightness() const
{
    return keyboard_brightness_;
}

bool CardputerBoard::isRTCReady() const
{
    return false;
}

bool CardputerBoard::isCharging() const
{
    return false;
}

int CardputerBoard::getBatteryLevel() const
{
    return -1;
}

uint16_t CardputerBoard::displayWidth() const
{
    return M5Cardputer.Display.width();
}

uint16_t CardputerBoard::displayHeight() const
{
    return M5Cardputer.Display.height();
}

void CardputerBoard::displayPushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* colors)
{
    M5Cardputer.Display.pushImage(x, y, w, h, colors);
}

bool CardputerBoard::isSDReady() const
{
    return sd_ready_;
}

bool CardputerBoard::initAudio(uint8_t& bclk, uint8_t& lrck, uint8_t& dout, int8_t& mclk)
{
    (void)mclk;
    auto boardType = M5.getBoard();
    bool ok = false;
    if (boardType == m5::board_t::board_M5CardputerADV) {
        ok = init_es8311();
    } else if (boardType == m5::board_t::board_M5Cardputer) {
        ok = init_aw88298();
    } else {
        ok = init_es8311();
        if (!ok) {
            ok = init_aw88298();
        }
    }

    bclk = kI2SBclk;
    lrck = kI2SLrck;
    dout = kI2SDout;
    return ok;
}

static bool map_char_to_lv_key(char c, uint32_t* key)
{
    if (!key) {
        return false;
    }
    if (c == '\n' || c == '\r') {
        *key = LV_KEY_ENTER;
        return true;
    }
    if (c == '\b') {
        *key = LV_KEY_BACKSPACE;
        return true;
    }
    if (c == ',') {
        *key = LV_KEY_LEFT;
        return true;
    }
    if (c == '.') {
        *key = LV_KEY_RIGHT;
        return true;
    }
    char lower = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    switch (lower) {
    case 'w':
    case 'k':
        *key = LV_KEY_UP;
        return true;
    case 's':
    case 'j':
        *key = LV_KEY_DOWN;
        return true;
    case 'a':
    case 'h':
        *key = LV_KEY_BACKSPACE;
        return true;
    case 'd':
    case 'l':
        *key = LV_KEY_ENTER;
        return true;
    case 'q':
        *key = LV_KEY_ESC;
        return true;
    default:
        break;
    }
    return false;
}

bool CardputerBoard::readKey(uint32_t* key)
{
    if (!keyboard_ready_) {
        return false;
    }

    M5Cardputer.update();

    auto& status = M5Cardputer.Keyboard.keysState();
    if (status.enter) {
        *key = LV_KEY_ENTER;
        return true;
    }
    if (status.del) {
        *key = LV_KEY_BACKSPACE;
        return true;
    }

    for (auto c : status.word) {
        if (map_char_to_lv_key(c, key)) {
            return true;
        }
    }

    return false;
}

#endif // defined(BOARD_CARDPUTER_ADV)
