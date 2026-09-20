#include "app/music_lights.h"

#include <Preferences.h>

namespace app::music_lights {
namespace {
constexpr char kPrefsNamespace[] = "lights";
constexpr char kEnabledKey[] = "music";

volatile bool s_enabled = true;
volatile uint8_t s_level = 0;
uint8_t s_envelope = 0;
uint8_t s_i2s_sample_count = 0;
uint16_t s_i2s_peak = 0;

uint16_t abs_sample(int16_t value) {
  // Do not overflow when a PCM sample happens to be INT16_MIN.
  return value == INT16_MIN ? 32768U
                            : static_cast<uint16_t>(value < 0 ? -value : value);
}

void publish_peak(uint16_t peak) {
  uint16_t target = 0;
  if (peak > 280) {
    target = static_cast<uint16_t>(((peak - 280U) * 100U) / 6200U);
  }
  if (target > 100) {
    target = 100;
  }
  if (target >= s_envelope) {
    s_envelope = static_cast<uint8_t>((s_envelope + target * 3U) / 4U);
  } else {
    s_envelope = static_cast<uint8_t>((s_envelope * 7U + target) / 8U);
  }
  s_level = s_envelope;
}
} // namespace

void init() {
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    s_enabled = prefs.getBool(kEnabledKey, true);
    prefs.end();
  }
  s_level = 0;
  s_envelope = 0;
  s_i2s_sample_count = 0;
  s_i2s_peak = 0;
}

bool enabled() { return s_enabled; }

void set_enabled(bool value) {
  if (s_enabled == value) {
    return;
  }
  s_enabled = value;
  if (!value) {
    s_level = 0;
    s_envelope = 0;
    s_i2s_sample_count = 0;
    s_i2s_peak = 0;
  }
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    prefs.putBool(kEnabledKey, value);
    prefs.end();
  }
}

void process_pcm(const int16_t *samples, uint16_t count) {
  if (!s_enabled || !samples || count == 0) {
    return;
  }

  // 64 samples are more than sufficient for a visible 25 FPS envelope and
  // make this callback negligible beside MP3 decoding and EQ processing.
  const uint16_t stride = count > 64 ? static_cast<uint16_t>(count / 64) : 1;
  uint32_t total = 0;
  uint16_t measured = 0;
  uint16_t peak = 0;
  for (uint16_t i = 0; i < count; i = static_cast<uint16_t>(i + stride)) {
    const uint16_t magnitude = abs_sample(samples[i]);
    total += magnitude;
    if (magnitude > peak) {
      peak = magnitude;
    }
    ++measured;
  }
  if (measured == 0) {
    return;
  }

  // Remove the decoder's quiet floor and map normal 16-bit music to 0..100.
  uint32_t average = total / measured;
  if (average > 180) {
    average -= 180;
  } else {
    average = 0;
  }
  uint16_t average_level = static_cast<uint16_t>((average * 100U) / 2600U);
  // Average energy is too conservative at the player's lower volume steps.
  // A sampled peak makes drum hits and transients visibly drive the lights.
  uint16_t peak_level = 0;
  if (peak > 450) {
    peak_level = static_cast<uint16_t>(((peak - 450U) * 100U) / 8500U);
  }
  uint16_t target = average_level > peak_level ? average_level : peak_level;
  if (target > 100) {
    target = 100;
  }

  // Fast attack, gentle release: readable musical motion without flicker.
  if (target >= s_envelope) {
    s_envelope = static_cast<uint8_t>((s_envelope + target * 3U) / 4U);
  } else {
    s_envelope = static_cast<uint8_t>((s_envelope * 7U + target) / 8U);
  }
  s_level = s_envelope;
}

void process_i2s_sample(uint32_t packed_stereo_sample) {
  if (!s_enabled) {
    return;
  }
  const int16_t left = static_cast<int16_t>(packed_stereo_sample & 0xFFFFU);
  const int16_t right = static_cast<int16_t>(packed_stereo_sample >> 16);
  uint16_t peak = abs_sample(left);
  const uint16_t right_peak = abs_sample(right);
  if (right_peak > peak) {
    peak = right_peak;
  }
  if (peak > s_i2s_peak) {
    s_i2s_peak = peak;
  }
  // At 44.1 kHz this publishes ~690 fresh peak samples per second; the UI
  // still limits physical LED I2C writes to 25 FPS.
  if (++s_i2s_sample_count >= 64) {
    publish_peak(s_i2s_peak);
    s_i2s_sample_count = 0;
    s_i2s_peak = 0;
  }
}

uint8_t level() { return s_level; }

} // namespace app::music_lights
