#pragma once

#include <Arduino.h>

namespace app::music_lights {

// The setting is stored independently from the board implementation so it
// remains available when the same SD card is moved between supported devices.
void init();
bool enabled();
void set_enabled(bool value);

// Called from the audio decoder callback.  This is deliberately allocation-
// free and only publishes a small envelope value for the foreground/UI core.
void process_pcm(const int16_t *samples, uint16_t count);
// Fallback for decoder paths which only expose a sample at the I2S boundary.
// The implementation internally decimates this 44.1 kHz callback.
void process_i2s_sample(uint32_t packed_stereo_sample);
uint8_t level();

} // namespace app::music_lights
