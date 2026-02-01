#pragma once

#include <stdint.h>

namespace app::eq
{
constexpr int kBandCount = 6;

struct Settings
{
    bool enabled = false;
    int8_t preamp_db = 0;
    int8_t band_db[kBandCount] = {};
};

void init();
Settings get_settings();
int8_t get_band(int band);
void set_band(int band, int8_t db);
int8_t get_preamp();
void set_preamp(int8_t db);
bool is_enabled();
void set_enabled(bool enabled);
void set_sample_rate(uint32_t sample_rate);
void process_block(int16_t* buffer, uint16_t frames, int channels, uint32_t sample_rate);

} // namespace app::eq
