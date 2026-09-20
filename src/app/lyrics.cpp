#include "app/lyrics.h"

#include <SD.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <esp_heap_caps.h>
#include <stdlib.h>
#include <string.h>

#include "app/network.h"

namespace app {
namespace {
constexpr size_t kLyricsTextBytes = 16384;
constexpr size_t kInputLineBytes = 384;
LyricsState s_lyrics;

void reset_state() {
  s_lyrics.line_count = 0;
  s_lyrics.text_used = 0;
  s_lyrics.track_index = -1;
  s_lyrics.source_found = false;
  s_lyrics.synchronized = false;
  ++s_lyrics.version;
  if (s_lyrics.text) {
    s_lyrics.text[0] = '\0';
  }
}

char *trim(char *value) {
  if (!value) {
    return value;
  }
  while (*value == ' ' || *value == '\t') {
    ++value;
  }
  char *end = value + strlen(value);
  while (end > value && (end[-1] == ' ' || end[-1] == '\t')) {
    *--end = '\0';
  }
  return value;
}

bool parse_timestamp(const char *tag, size_t len, uint32_t &time_ms) {
  if (!tag || len < 5 || tag[0] != '[' || tag[len - 1] != ']') {
    return false;
  }
  const char *colon = static_cast<const char *>(memchr(tag + 1, ':', len - 2));
  if (!colon) {
    return false;
  }
  char *minute_end = nullptr;
  const unsigned long minutes = strtoul(tag + 1, &minute_end, 10);
  if (minute_end != colon) {
    return false;
  }
  char second_buf[12] = {};
  const size_t second_len = static_cast<size_t>((tag + len - 1) - (colon + 1));
  if (second_len == 0 || second_len >= sizeof(second_buf)) {
    return false;
  }
  memcpy(second_buf, colon + 1, second_len);
  char *second_end = nullptr;
  const double seconds = strtod(second_buf, &second_end);
  if (second_end == second_buf || *second_end != '\0' || seconds < 0.0) {
    return false;
  }
  const double total_ms = (static_cast<double>(minutes) * 60.0 + seconds) * 1000.0;
  time_ms = total_ms > 4294967295.0 ? UINT32_MAX
                                     : static_cast<uint32_t>(total_ms + 0.5);
  return true;
}

void add_line(uint32_t time_ms, const char *text) {
  if (!text || !text[0] || !s_lyrics.lines || !s_lyrics.text ||
      s_lyrics.line_count >= kMaxLyricsLines) {
    return;
  }
  const size_t len = strlen(text);
  if (len + 1 > kLyricsTextBytes - s_lyrics.text_used) {
    return;
  }
  LyricLine &line = s_lyrics.lines[s_lyrics.line_count++];
  line.time_ms = time_ms;
  line.text_offset = s_lyrics.text_used;
  memcpy(s_lyrics.text + s_lyrics.text_used, text, len + 1);
  s_lyrics.text_used += static_cast<uint16_t>(len + 1);
}

void parse_line(char *line) {
  if (!line) {
    return;
  }
  char *cursor = trim(line);
  uint32_t stamps[8] = {};
  uint8_t stamp_count = 0;
  while (*cursor == '[' && stamp_count < sizeof(stamps) / sizeof(stamps[0])) {
    char *close = strchr(cursor, ']');
    if (!close) {
      break;
    }
    uint32_t time_ms = 0;
    if (!parse_timestamp(cursor, static_cast<size_t>(close - cursor + 1),
                         time_ms)) {
      break;
    }
    stamps[stamp_count++] = time_ms;
    cursor = close + 1;
  }
  cursor = trim(cursor);
  for (uint8_t i = 0; i < stamp_count; ++i) {
    add_line(stamps[i], cursor);
  }
}

void sort_lines() {
  for (uint16_t i = 1; i < s_lyrics.line_count; ++i) {
    LyricLine value = s_lyrics.lines[i];
    uint16_t j = i;
    while (j > 0 && s_lyrics.lines[j - 1].time_ms > value.time_ms) {
      s_lyrics.lines[j] = s_lyrics.lines[j - 1];
      --j;
    }
    s_lyrics.lines[j] = value;
  }
}

String lrc_path_for(const char *audio_path) {
  String path = audio_path ? String(audio_path) : String();
  const int slash = path.lastIndexOf('/');
  const int dot = path.lastIndexOf('.');
  if (dot > slash) {
    path.remove(dot);
  }
  path += ".lrc";
  return path;
}

String cache_path_for(const char *audio_path) {
  uint32_t hash = 2166136261UL;
  for (const char *cursor = audio_path; cursor && *cursor; ++cursor) {
    hash ^= static_cast<uint8_t>(*cursor);
    hash *= 16777619UL;
  }
  char filename[32] = {};
  snprintf(filename, sizeof(filename), "/lyrics/%08lx.lrc",
           static_cast<unsigned long>(hash));
  return String(filename);
}

bool load_file(const String &path) {
  File file = SD.open(path, FILE_READ);
  if (!file) {
    return false;
  }
  s_lyrics.source_found = true;
  char line[kInputLineBytes] = {};
  size_t used = 0;
  while (file.available()) {
    const int value = file.read();
    if (value < 0) {
      break;
    }
    if (value == '\r') {
      continue;
    }
    if (value == '\n') {
      line[used] = '\0';
      parse_line(line);
      used = 0;
    } else if (used + 1 < sizeof(line)) {
      line[used++] = static_cast<char>(value);
    }
  }
  if (used > 0) {
    line[used] = '\0';
    parse_line(line);
  }
  file.close();
  return true;
}

String url_encode(const char *value) {
  String encoded;
  static constexpr char kHex[] = "0123456789ABCDEF";
  for (const uint8_t *cursor = reinterpret_cast<const uint8_t *>(value ? value : "");
       *cursor; ++cursor) {
    const uint8_t ch = *cursor;
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.') {
      encoded += static_cast<char>(ch);
    } else {
      encoded += '%';
      encoded += kHex[ch >> 4];
      encoded += kHex[ch & 0x0f];
    }
  }
  return encoded;
}

String json_string_field(const String &json, const char *field) {
  const String marker = String("\"") + field + "\":\"";
  const int start = json.indexOf(marker);
  if (start < 0) {
    return "";
  }
  String value;
  bool escaped = false;
  for (int i = start + marker.length(); i < json.length(); ++i) {
    const char ch = json[i];
    if (escaped) {
      if (ch == 'n') value += '\n';
      else if (ch == 'r') value += '\r';
      else if (ch == 't') value += '\t';
      else value += ch;
      escaped = false;
    } else if (ch == '\\') {
      escaped = true;
    } else if (ch == '"') {
      break;
    } else {
      value += ch;
    }
  }
  return value;
}
} // namespace

void lyrics_init() {
  if (s_lyrics.lines && s_lyrics.text) {
    return;
  }
#if defined(BOARD_HAS_PSRAM)
  s_lyrics.lines = static_cast<LyricLine *>(heap_caps_malloc(
      sizeof(LyricLine) * kMaxLyricsLines, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  s_lyrics.text = static_cast<char *>(heap_caps_malloc(
      kLyricsTextBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
#endif
  if (!s_lyrics.lines) {
    s_lyrics.lines = static_cast<LyricLine *>(
        malloc(sizeof(LyricLine) * kMaxLyricsLines));
  }
  if (!s_lyrics.text) {
    s_lyrics.text = static_cast<char *>(malloc(kLyricsTextBytes));
  }
  reset_state();
}

void lyrics_load_for_track(int track_index, const char *audio_path) {
  lyrics_init();
  reset_state();
  s_lyrics.track_index = track_index;
  if (!audio_path || !audio_path[0] || !s_lyrics.lines || !s_lyrics.text) {
    return;
  }
  const String sidecar_path = lrc_path_for(audio_path);
  const String cache_path = cache_path_for(audio_path);
  if (!load_file(sidecar_path) && !load_file(cache_path)) {
    return;
  }
  sort_lines();
  s_lyrics.synchronized = s_lyrics.line_count > 0;
  Serial.printf("[LYRICS] lines=%u bytes=%u\n",
                static_cast<unsigned>(s_lyrics.line_count),
                static_cast<unsigned>(s_lyrics.text_used));
}

bool lyrics_download_for_track(int track_index, const char *audio_path,
                               const char *title, const char *artist,
                               const char *album, uint32_t duration_seconds) {
  if (!audio_path || !title || !title[0] || !artist || !artist[0] ||
      !network::connected()) {
    return false;
  }
  String url = "https://lrclib.net/api/get?track_name=" + url_encode(title) +
               "&artist_name=" + url_encode(artist);
  if (album && album[0]) {
    url += "&album_name=" + url_encode(album);
  }
  if (duration_seconds > 0) {
    url += "&duration=" + String(duration_seconds);
  }
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (!http.begin(client, url)) {
    return false;
  }
  // This is an explicit foreground operation.  Keep the failure path bounded
  // rather than leaving the player paused for a long captive-portal timeout.
  http.setTimeout(8000);
  http.addHeader("User-Agent", "LoFiBox/1.0 (https://github.com/vicliu624/LoFiBox)");
  const int status = http.GET();
  const String response = status == HTTP_CODE_OK ? http.getString() : String();
  http.end();
  const String synced = json_string_field(response, "syncedLyrics");
  if (synced.isEmpty()) {
    Serial.printf("[LYRICS] LRCLIB status=%d no synced lyrics\n", status);
    return false;
  }
  SD.mkdir("/lyrics");
  const String cache_path = cache_path_for(audio_path);
  if (SD.exists(cache_path)) {
    SD.remove(cache_path);
  }
  File cache = SD.open(cache_path, FILE_WRITE);
  if (!cache) {
    return false;
  }
  const size_t written = cache.print(synced);
  cache.close();
  if (written != synced.length()) {
    SD.remove(cache_path);
    return false;
  }
  lyrics_load_for_track(track_index, audio_path);
  Serial.printf("[LYRICS] downloaded %s\n", cache_path.c_str());
  return lyrics_state().synchronized;
}

const LyricsState &lyrics_state() { return s_lyrics; }

const char *lyrics_line_text(int line_index) {
  if (!s_lyrics.text || line_index < 0 || line_index >= s_lyrics.line_count) {
    return "";
  }
  return s_lyrics.text + s_lyrics.lines[line_index].text_offset;
}

int lyrics_active_line(uint32_t playback_time_ms) {
  if (!s_lyrics.synchronized || s_lyrics.line_count == 0) {
    return -1;
  }
  int active = 0;
  for (uint16_t i = 1; i < s_lyrics.line_count; ++i) {
    if (s_lyrics.lines[i].time_ms > playback_time_ms) {
      break;
    }
    active = i;
  }
  return active;
}

} // namespace app
