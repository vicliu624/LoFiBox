#pragma once

#include <Arduino.h>
#include <FS.h>

namespace app {

enum class CoverFormat : uint8_t {
  Unknown = 0,
  Jpeg,
  Png,
  Bmp,
};

constexpr int kMaxTracks = 512;
constexpr int kMaxArtists = 128;
constexpr int kMaxAlbums = 256;
constexpr int kMaxGenres = 64;
constexpr int kMaxComposers = 64;
constexpr int kMaxPlaylistTracks = 256;

struct StringPool {
  char *data = nullptr;
  size_t capacity = 0;
  size_t used = 0;
  bool in_psram = false;

  void init(size_t cap);
  void reset();
  const char *store(const String &s);
  const char *store_cstr(const char *s);
};

struct TrackInfo {
  const char *path = "";
  const char *title = "";
  const char *artist = "";
  const char *album = "";
  const char *genre = "";
  const char *composer = "";
  uint32_t cover_pos = 0;
  uint32_t cover_len = 0;
  CoverFormat cover_format = CoverFormat::Unknown;
  uint32_t duration_sec = 0;
  uint32_t added_time = 0;
  uint32_t play_count = 0;
  uint32_t last_played = 0;
};

struct AlbumInfo {
  const char *name = "";
  const char *artist = "";
};

struct Library {
  TrackInfo tracks[kMaxTracks];
  int track_count = 0;

  const char *artists[kMaxArtists] = {};
  int artist_count = 0;

  AlbumInfo albums[kMaxAlbums];
  int album_count = 0;

  const char *genres[kMaxGenres] = {};
  int genre_count = 0;

  const char *composers[kMaxComposers] = {};
  int composer_count = 0;

  StringPool pool{};
  bool scanned = false;
};

void library_reset(Library &lib);
bool library_scan(Library &lib, fs::FS &fs, const char *root_dir, uint8_t depth,
                  int max_files = kMaxTracks, bool read_tags = true,
                  void (*tick)() = nullptr);

int library_find_artist(const Library &lib, const String &name);
int library_find_album(const Library &lib, const String &name,
                       const String &artist);

int library_tracks_for_artist(const Library &lib, const String &artist,
                              int *out, int max);
int library_tracks_for_album(const Library &lib, const String &artist,
                             const String &album, int *out, int max);
int library_tracks_for_genre(const Library &lib, const String &genre, int *out,
                             int max);
int library_tracks_for_composer(const Library &lib, const String &composer,
                                int *out, int max);

int library_albums_for_artist(const Library &lib, const String &artist,
                              int *out, int max);

} // namespace app
