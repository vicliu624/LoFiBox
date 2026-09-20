#include "app/player.h"

#include <Arduino.h>
#include <Audio.h>
#include <FS.h>
#include <Preferences.h>
#include <SD.h>
#include <cstring>

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#endif

#include "app/lyrics.h"
#include "board/BoardBase.h"

namespace app {
namespace {
static Audio s_audio;
static Library *s_library = nullptr;
static PlayerState *s_state = nullptr;
static uint32_t s_last_sample_rate = 0;
static uint32_t s_clock_seconds = 0;
static uint32_t s_clock_mark_ms = 0;
static uint32_t s_last_output_detect_ms = 0;
static BoardBase::AudioOutput s_active_output = BoardBase::AudioOutput::Speaker;

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
// Core2 redraws are synchronous SPI transfers. Keep decoding on the other
// ESP32 core so a focused row, a page transition, or a touch repaint cannot
// drain the I2S DMA queue. A recursive mutex is required because Audio::loop()
// can invoke audio_eof_mp3(), which immediately starts the next track.
static SemaphoreHandle_t s_audio_mutex = nullptr;
static TaskHandle_t s_audio_task = nullptr;
constexpr uint32_t kAudioTaskStackWords = 8192;
constexpr UBaseType_t kAudioTaskPriority = 3;

class AudioGuard {
public:
  AudioGuard() {
    if (s_audio_mutex) {
      xSemaphoreTakeRecursive(s_audio_mutex, portMAX_DELAY);
      locked_ = true;
    }
  }
  ~AudioGuard() {
    if (locked_) {
      xSemaphoreGiveRecursive(s_audio_mutex);
    }
  }

private:
  bool locked_ = false;
};

void audio_service_task(void *) {
  for (;;) {
    if (s_state && s_state->is_playing && !s_state->paused) {
      {
        AudioGuard lock;
        if (s_state->is_playing && !s_state->paused) {
          s_audio.loop();
        }
      }
      // Yield to the ESP32 system tasks without leaving a multi-millisecond
      // gap in decoder servicing.
      taskYIELD();
    } else {
      vTaskDelay(1);
    }
  }
}

bool audio_service_is_running() { return s_audio_task != nullptr; }
#else
class AudioGuard {};
bool audio_service_is_running() { return false; }
#endif

constexpr char kAudioPrefsNamespace[] = "audio";
constexpr char kAudioOutputKey[] = "output";

constexpr size_t kCoverScanMax = 16384;
constexpr size_t kCoverChunkSize = 512;
constexpr size_t kCoverMaxBytes = 512 * 1024;

static bool match_sig(const uint8_t *buf, size_t len, const uint8_t *sig,
                      size_t siglen) {
  if (len < siglen) {
    return false;
  }
  return memcmp(buf, sig, siglen) == 0;
}

static bool same_str(const char *a, const char *b) {
  if (!a) {
    a = "";
  }
  if (!b) {
    b = "";
  }
  return strcmp(a, b) == 0;
}

static BoardBase::AudioOutput resolve_audio_output(const PlayerState &state) {
  if (state.audio_output_mode == AudioOutputMode::Headphones) {
    return BoardBase::AudioOutput::Headphones;
  }
  if (state.audio_output_mode == AudioOutputMode::Speaker) {
    return BoardBase::AudioOutput::Speaker;
  }
  return board.headphonesInserted() ? BoardBase::AudioOutput::Headphones
                                    : BoardBase::AudioOutput::Speaker;
}

static void apply_audio_output(PlayerState &state, bool force = false) {
  if (!board.supportsAudioOutputSelection()) {
    return;
  }
  const BoardBase::AudioOutput next = resolve_audio_output(state);
  if (!force && next == s_active_output) {
    return;
  }

  uint8_t bclk = 0;
  uint8_t lrck = 0;
  uint8_t dout = 0;
  int8_t mclk = -1;
  if (!board.getAudioOutputPinout(next, bclk, lrck, dout, mclk)) {
    return;
  }

  // setPinout only remaps the running I2S peripheral; it does not recreate
  // the decoder or reopen the file, so a jack insertion does not restart the
  // current track.
  {
    AudioGuard lock;
    board.setAudioOutput(next);
    if (!s_audio.setPinout(bclk, lrck, dout, I2S_PIN_NO_CHANGE, mclk)) {
      return;
    }
  }
  s_active_output = next;
  const bool headphones = next == BoardBase::AudioOutput::Headphones;
  if (state.headphones_active != headphones || force) {
    state.headphones_active = headphones;
    ++state.audio_output_version;
  }
}

static void update_auto_audio_output(PlayerState &state) {
  if (state.audio_output_mode != AudioOutputMode::Auto ||
      !board.supportsAudioOutputSelection()) {
    return;
  }
  const uint32_t now = millis();
  if (now - s_last_output_detect_ms < 500) {
    return;
  }
  s_last_output_detect_ms = now;
  apply_audio_output(state);
}

static CoverFormat detect_cover_format(File &file, size_t pos) {
  if (!file) {
    return CoverFormat::Unknown;
  }
  uint8_t buf[8] = {};
  size_t saved = file.position();
  file.seek(pos);
  size_t rd = file.read(buf, sizeof(buf));
  file.seek(saved);
  if (rd >= 2) {
    const uint8_t sig_jpg[2] = {0xFF, 0xD8};
    if (match_sig(buf, rd, sig_jpg, sizeof(sig_jpg))) {
      return CoverFormat::Jpeg;
    }
  }
  if (rd >= 8) {
    const uint8_t sig_png[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    if (match_sig(buf, rd, sig_png, sizeof(sig_png))) {
      return CoverFormat::Png;
    }
  }
  if (rd >= 2) {
    const uint8_t sig_bmp[2] = {'B', 'M'};
    if (match_sig(buf, rd, sig_bmp, sizeof(sig_bmp))) {
      return CoverFormat::Bmp;
    }
  }
  return CoverFormat::Unknown;
}

static bool find_cover_start(File &file, size_t pos, size_t size,
                             size_t &image_pos, CoverFormat &fmt) {
  const uint8_t sig_jpg[2] = {0xFF, 0xD8};
  const uint8_t sig_png[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  const uint8_t sig_bmp[2] = {'B', 'M'};

  image_pos = pos;
  fmt = CoverFormat::Unknown;

  size_t max_scan = size < kCoverScanMax ? size : kCoverScanMax;
  size_t scanned = 0;
  uint8_t buf[kCoverChunkSize];

  file.seek(pos);
  while (scanned < max_scan) {
    size_t to_read = max_scan - scanned;
    if (to_read > sizeof(buf)) {
      to_read = sizeof(buf);
    }
    size_t rd = file.read(buf, to_read);
    if (rd == 0) {
      break;
    }

    for (size_t i = 0; i + 1 < rd; ++i) {
      if (buf[i] == sig_jpg[0] && buf[i + 1] == sig_jpg[1]) {
        image_pos = pos + scanned + i;
        fmt = CoverFormat::Jpeg;
        return true;
      }
    }

    for (size_t i = 0; i + sizeof(sig_png) <= rd; ++i) {
      if (match_sig(&buf[i], rd - i, sig_png, sizeof(sig_png))) {
        image_pos = pos + scanned + i;
        fmt = CoverFormat::Png;
        return true;
      }
    }

    for (size_t i = 0; i + 1 < rd; ++i) {
      if (buf[i] == sig_bmp[0] && buf[i + 1] == sig_bmp[1]) {
        image_pos = pos + scanned + i;
        fmt = CoverFormat::Bmp;
        return true;
      }
    }

    scanned += rd;
  }

  return false;
}

static void reset_cover(PlayerState &state) {
  state.cover_ready = false;
  state.cover_path = "";
  state.cover_track_index = -1;
  state.cover_pos = 0;
  state.cover_len = 0;
  state.cover_format = CoverFormat::Unknown;
  state.cover_version++;
}

static void start_track(int index) {
  if (!s_library || !s_state) {
    return;
  }
  if (index < 0 || index >= s_library->track_count) {
    return;
  }

  TrackInfo &track = s_library->tracks[index];
  s_state->current_index = index;
  s_state->is_playing = true;
  s_state->paused = false;
  board.setAudioActive(true);
  track.play_count++;
  track.last_played = millis() / 1000;
  reset_cover(*s_state);
  lyrics_load_for_track(index, track.path);
  s_clock_seconds = 0;
  s_clock_mark_ms = millis();
  if (track.cover_len > 0 && track.cover_format != CoverFormat::Unknown) {
    s_state->cover_pos = track.cover_pos;
    s_state->cover_len = track.cover_len;
    s_state->cover_format = track.cover_format;
    s_state->cover_track_index = index;
    s_state->cover_ready = true;
  }

  {
    AudioGuard lock;
    s_audio.stopSong();
    s_audio.connecttoFS(SD, track.path ? track.path : "");
  }
}

static void pick_next(bool forward) {
  if (!s_library || !s_state || s_library->track_count == 0) {
    return;
  }

  int next_index = s_state->current_index;
  if (s_state->mode == PlaybackMode::RepeatOne) {
    next_index = (s_state->current_index >= 0) ? s_state->current_index : 0;
  } else if (s_state->mode == PlaybackMode::Shuffle) {
    next_index = random(0, s_library->track_count);
  } else {
    if (next_index < 0) {
      next_index = 0;
    } else {
      next_index += forward ? 1 : -1;
      if (next_index >= s_library->track_count) {
        next_index = 0;
      } else if (next_index < 0) {
        next_index = s_library->track_count - 1;
      }
    }
  }

  start_track(next_index);
}

static void update_from_id3(const char *info) {
  if (!s_library || !s_state || s_state->current_index < 0 || !info) {
    return;
  }

  String s(info);
  s.trim();
  if (s.length() == 0) {
    return;
  }

  auto assignKV = [&](const char *key, const char *&out) {
    int n = strlen(key);
    if (s.startsWith(key)) {
      int pos = n;
      if (pos < s.length() && (s[pos] == ':' || s[pos] == '=')) {
        pos++;
      }
      String v = s.substring(pos);
      v.trim();
      if (v.length() > 0 && !same_str(out, v.c_str())) {
        out = s_library->pool.store(v);
        return true;
      }
    }
    return false;
  };

  TrackInfo &track = s_library->tracks[s_state->current_index];
  bool changed = false;
  if (!assignKV("Title", track.title)) {
    changed |= assignKV("TIT2", track.title);
  } else {
    changed = true;
  }
  if (!assignKV("Artist", track.artist)) {
    changed |= assignKV("TPE1", track.artist);
  } else {
    changed = true;
  }
  if (!assignKV("Album", track.album)) {
    changed |= assignKV("TALB", track.album);
  } else {
    changed = true;
  }
  if (!assignKV("Genre", track.genre)) {
    changed |= assignKV("TCON", track.genre);
  } else {
    changed = true;
  }
  if (!assignKV("Composer", track.composer)) {
    changed |= assignKV("TCOM", track.composer);
  } else {
    changed = true;
  }

  if (changed) {
    s_state->meta_version++;
  }
}
} // namespace

static void handle_id3(const char *info) { update_from_id3(info); }

static void handle_eof() {
  if (!s_state) {
    return;
  }
  pick_next(true);
}

static void handle_id3_image(File &file, size_t pos, size_t size) {
  if (!s_state || size == 0) {
    return;
  }

  size_t saved_pos = file.position();
  size_t image_pos = pos;
  CoverFormat fmt = CoverFormat::Unknown;
  fmt = detect_cover_format(file, image_pos);
  bool found = (fmt != CoverFormat::Unknown);
  if (!found) {
    found = find_cover_start(file, pos, size, image_pos, fmt);
  }
  if (!found || fmt == CoverFormat::Unknown) {
    Serial.printf("[ID3] cover not found pos=%u size=%u\n",
                  static_cast<unsigned>(pos), static_cast<unsigned>(size));
    file.seek(saved_pos);
    return;
  }

  size_t image_len = size - (image_pos - pos);
  if (image_len == 0) {
    Serial.printf("[ID3] cover length zero pos=%u size=%u\n",
                  static_cast<unsigned>(pos), static_cast<unsigned>(size));
    file.seek(saved_pos);
    return;
  }

  Serial.printf("[ID3] cover pos=%u len=%u fmt=%d\n",
                static_cast<unsigned>(image_pos),
                static_cast<unsigned>(image_len), static_cast<int>(fmt));
  s_state->cover_ready = true;
  s_state->cover_format = fmt;
  s_state->cover_pos = static_cast<uint32_t>(image_pos);
  s_state->cover_len = static_cast<uint32_t>(image_len);
  s_state->cover_track_index = s_state->current_index;
  s_state->cover_version++;

  file.seek(saved_pos);
}

void player_init(PlayerState &state, Library &lib) {
  s_library = &lib;
  s_state = &state;
  s_last_sample_rate = 0;
  s_clock_seconds = 0;
  s_clock_mark_ms = millis();
  s_last_output_detect_ms = 0;
  Preferences prefs;
  // Open read/write on first boot too: a read-only Preferences namespace
  // cannot be opened until it already exists, which would make a fresh Core2
  // emit an error before it can use the default Auto route.
  if (prefs.begin(kAudioPrefsNamespace, false)) {
    const uint8_t saved = prefs.getUChar(
        kAudioOutputKey, static_cast<uint8_t>(AudioOutputMode::Auto));
    if (saved <= static_cast<uint8_t>(AudioOutputMode::Headphones)) {
      state.audio_output_mode = static_cast<AudioOutputMode>(saved);
    }
    prefs.end();
  }
  lyrics_init();
  state.cover_ready = false;
  state.cover_path = "";
  state.cover_version = 0;
  state.meta_version = 0;
  state.cover_track_index = -1;
  state.cover_pos = 0;
  state.cover_len = 0;
  state.cover_format = CoverFormat::Unknown;

  uint8_t bclk = 0;
  uint8_t lrck = 0;
  uint8_t dout = 0;
  int8_t mclk = I2S_PIN_NO_CHANGE;

  board.initAudio(bclk, lrck, dout, mclk);
  {
    AudioGuard lock;
    s_audio.setPinout(bclk, lrck, dout, I2S_PIN_NO_CHANGE, mclk);
    s_audio.setVolume(state.volume);
  }
  apply_audio_output(state, true);

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
  if (!s_audio_mutex) {
    s_audio_mutex = xSemaphoreCreateRecursiveMutex();
  }
  if (s_audio_mutex && !s_audio_task) {
    const BaseType_t started = xTaskCreatePinnedToCore(
        audio_service_task, "lofi_audio", kAudioTaskStackWords, nullptr,
        kAudioTaskPriority, &s_audio_task, 0);
    if (started != pdPASS) {
      s_audio_task = nullptr;
      Serial.println("[CORE2 AUDIO] failed to start audio task");
    } else {
      Serial.println("[CORE2 AUDIO] decoder task pinned to core 0");
    }
  }
#endif
}

void player_loop(PlayerState &state) {
  update_auto_audio_output(state);
  if (!state.is_playing || state.paused) {
    return;
  }
  if (!audio_service_is_running()) {
    AudioGuard lock;
    s_audio.loop();
  }
  uint32_t current_seconds = 0;
  uint32_t sample_rate = 0;
  {
    AudioGuard lock;
    current_seconds = s_audio.getAudioCurrentTime();
    sample_rate = s_audio.getSampleRate();
  }
  if (current_seconds != s_clock_seconds) {
    s_clock_seconds = current_seconds;
    s_clock_mark_ms = millis();
  }
  if (sample_rate != 0 && sample_rate != s_last_sample_rate) {
    board.setAudioSampleRate(sample_rate);
    s_last_sample_rate = sample_rate;
  }
}

void player_play(PlayerState &state, int track_index) {
  (void)state;
  start_track(track_index);
}

void player_toggle_pause(PlayerState &state) {
  if (!state.is_playing) {
    start_track(state.current_index >= 0 ? state.current_index : 0);
    return;
  }
  state.paused = !state.paused;
  {
    AudioGuard lock;
    s_audio.pauseResume();
  }
  board.setAudioActive(!state.paused);
}

void player_next(PlayerState &state) {
  (void)state;
  pick_next(true);
}

void player_prev(PlayerState &state) {
  (void)state;
  pick_next(false);
}

void player_stop(PlayerState &state) {
  {
    AudioGuard lock;
    s_audio.stopSong();
  }
  state.is_playing = false;
  state.paused = false;
  board.setAudioActive(false);
}

uint8_t player_get_volume(const PlayerState &state) { return state.volume; }

void player_set_volume(PlayerState &state, uint8_t volume) {
  if (volume > 21) {
    volume = 21;
  }
  state.volume = volume;
  {
    AudioGuard lock;
    s_audio.setVolume(volume);
  }
}

const char *player_audio_output_mode_name(AudioOutputMode mode) {
  switch (mode) {
  case AudioOutputMode::Speaker:
    return "Core2 speaker";
  case AudioOutputMode::Headphones:
    return "Headphones";
  case AudioOutputMode::Auto:
  default:
    return "Auto";
  }
}

String player_audio_output_label(const PlayerState &state) {
  if (state.audio_output_mode != AudioOutputMode::Auto) {
    return player_audio_output_mode_name(state.audio_output_mode);
  }
  return String("Auto: ") +
         (state.headphones_active ? "Headphones" : "Core2 speaker");
}

void player_set_audio_output_mode(PlayerState &state, AudioOutputMode mode) {
  if (mode > AudioOutputMode::Headphones) {
    mode = AudioOutputMode::Auto;
  }
  state.audio_output_mode = mode;
  Preferences prefs;
  if (prefs.begin(kAudioPrefsNamespace, false)) {
    prefs.putUChar(kAudioOutputKey, static_cast<uint8_t>(mode));
    prefs.end();
  }
  apply_audio_output(state, true);
}

uint32_t player_current_time() {
  AudioGuard lock;
  return s_audio.getAudioCurrentTime();
}

uint32_t player_current_time_ms() {
  uint32_t seconds = 0;
  {
    AudioGuard lock;
    seconds = s_audio.getAudioCurrentTime();
  }
  if (!s_state || s_state->paused || !s_state->is_playing) {
    return seconds * 1000UL;
  }
  if (seconds != s_clock_seconds) {
    s_clock_seconds = seconds;
    s_clock_mark_ms = millis();
  }
  const uint32_t fraction = millis() - s_clock_mark_ms;
  return seconds * 1000UL + (fraction > 999 ? 999 : fraction);
}

uint32_t player_duration() {
  AudioGuard lock;
  return s_audio.getAudioFileDuration();
}

uint32_t player_sample_rate() {
  AudioGuard lock;
  return s_audio.getSampleRate();
}

uint8_t player_channels() {
  AudioGuard lock;
  return s_audio.getChannels();
}

uint8_t player_bits_per_sample() {
  AudioGuard lock;
  return s_audio.getBitsPerSample();
}

} // namespace app

void audio_info(const char *info) { (void)info; }

void audio_id3data(const char *info) { app::handle_id3(info); }

void audio_id3image(File &file, const size_t pos, const size_t size) {
  app::handle_id3_image(file, pos, size);
}

void audio_eof_mp3(const char *info) {
  (void)info;
  app::handle_eof();
}
