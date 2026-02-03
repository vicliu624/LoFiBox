#include "app/player.h"

#include <Arduino.h>
#include <Audio.h>
#include <FS.h>
#include <SD.h>
#include <cstring>

#include "board/BoardBase.h"

namespace app {
namespace {
static Audio s_audio;
static Library *s_library = nullptr;
static PlayerState *s_state = nullptr;

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
  track.play_count++;
  track.last_played = millis() / 1000;
  reset_cover(*s_state);
  if (track.cover_len > 0 && track.cover_format != CoverFormat::Unknown) {
    s_state->cover_pos = track.cover_pos;
    s_state->cover_len = track.cover_len;
    s_state->cover_format = track.cover_format;
    s_state->cover_track_index = index;
    s_state->cover_ready = true;
  }

  s_audio.stopSong();
  s_audio.connecttoFS(SD, track.path ? track.path : "");
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
  s_audio.setPinout(bclk, lrck, dout, I2S_PIN_NO_CHANGE, mclk);
  s_audio.setVolume(state.volume);
}

void player_loop(PlayerState &state) {
  if (!state.is_playing || state.paused) {
    return;
  }
  s_audio.loop();
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
  s_audio.pauseResume();
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
  s_audio.stopSong();
  state.is_playing = false;
  state.paused = false;
}

uint8_t player_get_volume(const PlayerState &state) { return state.volume; }

void player_set_volume(PlayerState &state, uint8_t volume) {
  if (volume > 21) {
    volume = 21;
  }
  state.volume = volume;
  s_audio.setVolume(volume);
}

uint32_t player_current_time() { return s_audio.getAudioCurrentTime(); }

uint32_t player_duration() { return s_audio.getAudioFileDuration(); }

uint32_t player_sample_rate() { return s_audio.getSampleRate(); }

uint8_t player_channels() { return s_audio.getChannels(); }

uint8_t player_bits_per_sample() { return s_audio.getBitsPerSample(); }

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
