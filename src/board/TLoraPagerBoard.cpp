#include "TLoraPagerBoard.h"

#include <Wire.h>
#include <cctype>
#include <lvgl.h>
#include <SD.h>
#include <SPI.h>

#include "board/sd_utils.h"

#ifdef USING_INPUT_DEV_KEYBOARD
#include "LilyGoKeyboard.h"
#endif

namespace
{
constexpr uint8_t kIoAddr = 0x20;
constexpr uint8_t kEs8311Addr = 0x18;

#ifdef USING_INPUT_DEV_KEYBOARD
static LilyGoKeyboard kb;
static constexpr char keymap[4][10] = {
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
    {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n'},
    {'\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}};
static constexpr char symbol_map[4][10] = {
    {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
    {'*', '/', '+', '-', '=', ':', '\'', '"', '@', '\0'},
    {'\0', '_', '$', ';', '?', '!', ',', '.', '\0', '\0'},
    {' ', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'}};

static const LilyGoKeyboardConfigure_t keyboardConfig = {
    .kb_rows = 4,
    .kb_cols = 10,
    .current_keymap = &keymap[0][0],
    .current_symbol_map = &symbol_map[0][0],
    .symbol_key_value = 0x1E,
    .alt_key_value = 0x14,
    .caps_key_value = 0x1C,
    .caps_b_key_value = 0xFF,
    .char_b_value = 0x19,
    .backspace_value = 0x1D,
    .has_symbol_key = false};
#endif

bool i2c_write_reg(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(kEs8311Addr);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
}

bool init_es8311()
{
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
        if (!i2c_write_reg(entry.reg, entry.val)) {
            ok = false;
        }
    }
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

int8_t read_rotary_step()
{
    static uint8_t last_state = 0;
    static int8_t accum = 0;
    static uint32_t last_step_ms = 0;
    static int8_t last_dir = 0;
    const uint8_t a = digitalRead(ROTARY_A) ? 1 : 0;
    const uint8_t b = digitalRead(ROTARY_B) ? 1 : 0;
    const uint8_t state = static_cast<uint8_t>((a << 1) | b);
    static const int8_t table[16] = {
        0, -1, 1, 0,
        1, 0, 0, -1,
        -1, 0, 0, 1,
        0, 1, -1, 0
    };
    const uint8_t idx = static_cast<uint8_t>((last_state << 2) | state);
    const int8_t delta = table[idx];
    if (delta != 0) {
        accum += delta;
    }
    last_state = state;

    if (accum >= 2) {
        accum = 0;
        int8_t dir = 1;
        uint32_t now = millis();
        if (last_dir != 0 && dir != last_dir && (now - last_step_ms) < 80) {
            return 0;
        }
        last_step_ms = now;
        last_dir = dir;
        return dir;
    }
    if (accum <= -2) {
        accum = 0;
        int8_t dir = -1;
        uint32_t now = millis();
        if (last_dir != 0 && dir != last_dir && (now - last_step_ms) < 80) {
            return 0;
        }
        last_step_ms = now;
        last_dir = dir;
        return dir;
    }
    return 0;
}

bool read_rotary_press()
{
    static bool last_pressed = false;
    bool pressed = (digitalRead(ROTARY_C) == LOW);
    bool rising = pressed && !last_pressed;
    last_pressed = pressed;
    return rising;
}
} // namespace

uint32_t TLoraPagerBoard::begin(uint32_t disable_hw_init)
{
    (void)disable_hw_init;

    Wire.begin(SDA, SCL);

    pinMode(LORA_CS, OUTPUT);
    digitalWrite(LORA_CS, HIGH);
    pinMode(NFC_CS, OUTPUT);
    digitalWrite(NFC_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    pinMode(ROTARY_A, INPUT_PULLUP);
    pinMode(ROTARY_B, INPUT_PULLUP);
    pinMode(ROTARY_C, INPUT_PULLUP);

    if (io_.begin(Wire, kIoAddr)) {
#ifdef EXPANDS_GPIO_EN
        io_.pinMode(EXPANDS_GPIO_EN, OUTPUT);
        io_.digitalWrite(EXPANDS_GPIO_EN, HIGH);
#endif
#ifdef EXPANDS_SD_EN
        io_.pinMode(EXPANDS_SD_EN, OUTPUT);
        io_.digitalWrite(EXPANDS_SD_EN, HIGH);
#endif
#ifdef EXPANDS_AMP_EN
        io_.pinMode(EXPANDS_AMP_EN, OUTPUT);
        io_.digitalWrite(EXPANDS_AMP_EN, HIGH);
#endif
#ifdef EXPANDS_KB_EN
        io_.pinMode(EXPANDS_KB_EN, OUTPUT);
        io_.digitalWrite(EXPANDS_KB_EN, HIGH);
#endif
#ifdef EXPANDS_KB_RST
        io_.pinMode(EXPANDS_KB_RST, OUTPUT);
        io_.digitalWrite(EXPANDS_KB_RST, LOW);
        delay(10);
        io_.digitalWrite(EXPANDS_KB_RST, HIGH);
        delay(10);
#endif
    }

    backlight_.begin(DISP_BL);

    display_ready_ = display_.init(DISP_SCK, DISP_MISO, DISP_MOSI, DISP_CS, DISP_RST, DISP_DC, -1, 80);
    if (display_ready_) {
        display_.setRotation(0);
        backlight_.setBrightness(brightness_);
    }

#ifdef USING_INPUT_DEV_KEYBOARD
    kb.setPins(KB_BACKLIGHT);
    keyboard_ready_ = kb.begin(keyboardConfig, Wire, KB_INT);
#endif

    const int extra_cs[] = {LORA_CS, NFC_CS};
    sd_ready_ = sdutil::installSpiSd(SPI, SD_CS, 40000000, "/sd", extra_cs, sizeof(extra_cs) / sizeof(extra_cs[0]));

    return 0;
}

void TLoraPagerBoard::wakeUp() {}

void TLoraPagerBoard::handlePowerButton() {}

void TLoraPagerBoard::softwareShutdown() {}

void TLoraPagerBoard::setBrightness(uint8_t level)
{
    brightness_ = level;
    backlight_.setBrightness(level);
}

uint8_t TLoraPagerBoard::getBrightness() const
{
    return backlight_.getBrightness();
}

bool TLoraPagerBoard::hasKeyboard() const
{
    return keyboard_ready_;
}

void TLoraPagerBoard::keyboardSetBrightness(uint8_t level)
{
    keyboard_brightness_ = level;
#ifdef USING_INPUT_DEV_KEYBOARD
    if (keyboard_ready_) {
        kb.setBrightness(level);
    }
#endif
}

uint8_t TLoraPagerBoard::keyboardGetBrightness() const
{
    if (keyboard_ready_) {
#ifdef USING_INPUT_DEV_KEYBOARD
        return kb.getBrightness();
#endif
    }
    return keyboard_brightness_;
}

bool TLoraPagerBoard::isRTCReady() const
{
    return false;
}

bool TLoraPagerBoard::isCharging() const
{
    return false;
}

int TLoraPagerBoard::getBatteryLevel() const
{
    return -1;
}

bool TLoraPagerBoard::readKey(uint32_t* key)
{
    if (read_rotary_press()) {
        if (key) {
            *key = LV_KEY_ENTER;
        }
        return true;
    }
    int8_t step = read_rotary_step();
    if (step != 0) {
        if (key) {
            *key = (step > 0) ? LV_KEY_UP : LV_KEY_DOWN;
        }
        return true;
    }
#ifdef USING_INPUT_DEV_KEYBOARD
    if (!keyboard_ready_) {
        return false;
    }
    char c = '\0';
    int state = kb.getKey(&c);
    if (state != KB_PRESSED) {
        return false;
    }
    return map_char_to_lv_key(c, key);
#else
    (void)key;
    return false;
#endif
}

uint16_t TLoraPagerBoard::displayWidth() const
{
    return display_._width;
}

uint16_t TLoraPagerBoard::displayHeight() const
{
    return display_._height;
}

void TLoraPagerBoard::displayPushColors(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* colors)
{
    display_.pushColors(x, y, w, h, const_cast<uint16_t*>(colors));
}

bool TLoraPagerBoard::isSDReady() const
{
    return sd_ready_;
}

bool TLoraPagerBoard::initAudio(uint8_t& bclk, uint8_t& lrck, uint8_t& dout, int8_t& mclk)
{
    Wire.begin(SDA, SCL);
    bool ok = init_es8311();

#ifdef EXPANDS_AMP_EN
    io_.digitalWrite(EXPANDS_AMP_EN, HIGH);
#endif

    bclk = I2S_SCK;
    lrck = I2S_WS;
    dout = I2S_SDOUT;
    mclk = I2S_MCLK;
    return ok;
}
