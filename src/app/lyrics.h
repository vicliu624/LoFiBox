#pragma once

#include <Arduino.h>

namespace app {

constexpr uint16_t kMaxLyricsLines = 192;

struct LyricLine {
  uint32_t time_ms = 0;
  uint16_t text_offset = 0;
};

struct LyricsState {
  LyricLine *lines = nullptr;
  char *text = nullptr;
  uint16_t line_count = 0;
  uint16_t text_used = 0;
  int track_index = -1;
  uint32_t version = 0;
  bool source_found = false;
  bool synchronized = false;
};

void lyrics_init();
void lyrics_load_for_track(int track_index, const char *audio_path);
bool lyrics_download_for_track(int track_index, const char *audio_path,
                               const char *title, const char *artist,
                               const char *album, uint32_t duration_seconds);
const LyricsState &lyrics_state();
const char *lyrics_line_text(int line_index);
int lyrics_active_line(uint32_t playback_time_ms);

} // namespace app
