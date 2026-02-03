#include "app/library.h"

#include <SD.h>
#include <cstring>
#include <esp_heap_caps.h>

namespace app {
namespace {
constexpr size_t kStringPoolSize = 512 * 1024;
const char *kUnknownArtist = "Unknown Artist";
const char *kUnknownAlbum = "Unknown Album";
const char *kUnknownGenre = "Unknown Genre";
const char *kUnknownComposer = "Unknown Composer";

struct TagInfo {
  String title;
  String artist;
  String album;
  String genre;
  String composer;
  uint32_t cover_pos = 0;
  uint32_t cover_len = 0;
  CoverFormat cover_format = CoverFormat::Unknown;
};

static const char *empty_or(const char *value, const char *fallback) {
  if (value && value[0] != '\0') {
    return value;
  }
  return fallback ? fallback : "";
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

static uint32_t read_u32_be(const uint8_t *data) {
  return (static_cast<uint32_t>(data[0]) << 24) |
         (static_cast<uint32_t>(data[1]) << 16) |
         (static_cast<uint32_t>(data[2]) << 8) | static_cast<uint32_t>(data[3]);
}

static uint32_t read_syncsafe(const uint8_t *data) {
  return (static_cast<uint32_t>(data[0] & 0x7F) << 21) |
         (static_cast<uint32_t>(data[1] & 0x7F) << 14) |
         (static_cast<uint32_t>(data[2] & 0x7F) << 7) |
         static_cast<uint32_t>(data[3] & 0x7F);
}

static String basename_no_ext(const String &path) {
  int slash = path.lastIndexOf('/');
  String name = (slash >= 0) ? path.substring(slash + 1) : path;
  int dot = name.lastIndexOf('.');
  if (dot > 0) {
    name = name.substring(0, dot);
  }
  name.trim();
  return name;
}

static String parent_dir(const String &path, int levels_up) {
  String dir = path;
  for (int i = 0; i < levels_up; ++i) {
    int slash = dir.lastIndexOf('/');
    if (slash <= 0) {
      return "";
    }
    dir = dir.substring(0, slash);
  }
  int last = dir.lastIndexOf('/');
  if (last < 0) {
    return dir;
  }
  return dir.substring(last + 1);
}

static void append_utf8(String &out, uint32_t codepoint) {
  if (codepoint <= 0x7F) {
    out += static_cast<char>(codepoint);
  } else if (codepoint <= 0x7FF) {
    out += static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F));
    out += static_cast<char>(0x80 | (codepoint & 0x3F));
  } else if (codepoint <= 0xFFFF) {
    out += static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F));
    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (codepoint & 0x3F));
  } else if (codepoint <= 0x10FFFF) {
    out += static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07));
    out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
    out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
    out += static_cast<char>(0x80 | (codepoint & 0x3F));
  }
}

static String decode_text_frame(uint8_t encoding, const uint8_t *data,
                                size_t len) {
  if (len == 0) {
    return String();
  }

  if (encoding == 3) {
    String out;
    out.reserve(len + 1);
    for (size_t i = 0; i < len; ++i) {
      if (data[i] == '\0') {
        break;
      }
      out += static_cast<char>(data[i]);
    }
    out.trim();
    return out;
  }

  if (encoding == 0) {
    String out;
    out.reserve(len + 1);
    for (size_t i = 0; i < len; ++i) {
      uint8_t b = data[i];
      if (b == 0x00) {
        break;
      }
      if (b < 0x80) {
        out += static_cast<char>(b);
      } else {
        append_utf8(out, static_cast<uint32_t>(b));
      }
    }
    out.trim();
    return out;
  }

  // UTF-16/UTF-16BE -> UTF-8
  bool big_endian = (encoding == 2);
  size_t start = 0;
  if (encoding == 1 && len >= 2) {
    if (data[0] == 0xFF && data[1] == 0xFE) {
      big_endian = false;
      start = 2;
    } else if (data[0] == 0xFE && data[1] == 0xFF) {
      big_endian = true;
      start = 2;
    } else {
      // Heuristic: look for zero byte position
      if (data[0] == 0x00 && data[1] != 0x00) {
        big_endian = true;
      } else if (data[1] == 0x00 && data[0] != 0x00) {
        big_endian = false;
      }
    }
  }

  if (start < len && ((len - start) & 1)) {
    len -= 1;
  }

  String out;
  out.reserve(len / 2 + 1);
  for (size_t i = start; i + 1 < len; i += 2) {
    uint16_t w = big_endian
                     ? (static_cast<uint16_t>(data[i]) << 8) | data[i + 1]
                     : (static_cast<uint16_t>(data[i + 1]) << 8) | data[i];
    if (w == 0x0000) {
      break;
    }
    if (w >= 0xD800 && w <= 0xDBFF && i + 3 < len) {
      uint16_t w2 =
          big_endian ? (static_cast<uint16_t>(data[i + 2]) << 8) | data[i + 3]
                     : (static_cast<uint16_t>(data[i + 3]) << 8) | data[i + 2];
      if (w2 >= 0xDC00 && w2 <= 0xDFFF) {
        uint32_t code = 0x10000 + (((w - 0xD800) << 10) | (w2 - 0xDC00));
        append_utf8(out, code);
        i += 2;
        continue;
      }
    }
    append_utf8(out, static_cast<uint32_t>(w));
  }
  out.trim();
  return out;
}

static void assign_if_empty(String &target, const String &value) {
  if (target.length() == 0 && value.length() > 0) {
    target = value;
  }
}

static bool read_id3_tags(fs::FS &fs, const String &path, TagInfo &out) {
  File f = fs.open(path, FILE_READ);
  if (!f) {
    return false;
  }

  uint8_t header[10] = {0};
  if (f.read(header, sizeof(header)) != sizeof(header)) {
    f.close();
    return false;
  }
  if (header[0] != 'I' || header[1] != 'D' || header[2] != '3') {
    f.close();
    return false;
  }

  uint8_t version = header[3];
  uint32_t tag_size = read_syncsafe(&header[6]);
  uint32_t pos = 10;

  while (pos + 10 <= tag_size + 10) {
    uint8_t frame_header[10] = {0};
    if (f.read(frame_header, sizeof(frame_header)) != sizeof(frame_header)) {
      break;
    }
    pos += 10;

    if (frame_header[0] == 0) {
      break;
    }

    char frame_id[5] = {0};
    memcpy(frame_id, frame_header, 4);

    uint32_t frame_size = (version == 4) ? read_syncsafe(&frame_header[4])
                                         : read_u32_be(&frame_header[4]);
    if (frame_size == 0 || pos + frame_size > tag_size + 10) {
      f.seek(tag_size + 10);
      break;
    }

    if (frame_id[0] == 'T') {
      uint8_t encoding = 0;
      if (frame_size > 0) {
        f.read(&encoding, 1);
        frame_size--;
        pos += 1;
      }
      const size_t max_len = 256;
      size_t read_len = frame_size > max_len ? max_len : frame_size;
      if ((encoding == 1 || encoding == 2) && (read_len & 1)) {
        read_len -= 1;
      }
      uint8_t buf[max_len];
      memset(buf, 0, sizeof(buf));
      if (read_len > 0) {
        f.read(buf, read_len);
        pos += read_len;
      }
      if (frame_size > read_len) {
        f.seek(f.position() + (frame_size - read_len));
        pos += frame_size - read_len;
      }

      String text = decode_text_frame(encoding, buf, read_len);
      if (strcmp(frame_id, "TIT2") == 0) {
        assign_if_empty(out.title, text);
      } else if (strcmp(frame_id, "TPE1") == 0) {
        assign_if_empty(out.artist, text);
      } else if (strcmp(frame_id, "TALB") == 0) {
        assign_if_empty(out.album, text);
      } else if (strcmp(frame_id, "TCON") == 0) {
        assign_if_empty(out.genre, text);
      } else if (strcmp(frame_id, "TCOM") == 0) {
        assign_if_empty(out.composer, text);
      }
    } else if (strcmp(frame_id, "APIC") == 0) {
      if (out.cover_len == 0 && frame_size > 0) {
        size_t frame_start = f.position();
        size_t consumed = 0;
        uint8_t encoding = 0;
        f.read(&encoding, 1);
        consumed = 1;

        String mime;
        while (consumed < frame_size) {
          uint8_t c = 0;
          if (f.read(&c, 1) != 1) {
            break;
          }
          consumed++;
          if (c == 0) {
            break;
          }
          if (mime.length() < 32) {
            mime += c;
          }
        }

        if (consumed < frame_size) {
          uint8_t pic_type = 0;
          f.read(&pic_type, 1);
          consumed++;
        }

        if (consumed < frame_size) {
          if (encoding == 0 || encoding == 3) {
            uint8_t c = 0;
            while (consumed < frame_size) {
              if (f.read(&c, 1) != 1) {
                break;
              }
              consumed++;
              if (c == 0) {
                break;
              }
            }
          } else {
            uint8_t prev = 0;
            uint8_t cur = 0;
            while (consumed + 1 < frame_size) {
              if (f.read(&cur, 1) != 1) {
                break;
              }
              consumed++;
              if (prev == 0 && cur == 0) {
                break;
              }
              prev = cur;
            }
          }
        }

        if (consumed < frame_size) {
          out.cover_pos = frame_start + consumed;
          out.cover_len = frame_size - consumed;
          String lower = mime;
          lower.toLowerCase();
          if (lower.indexOf("png") >= 0) {
            out.cover_format = CoverFormat::Png;
          } else if (lower.indexOf("jpeg") >= 0 || lower.indexOf("jpg") >= 0) {
            out.cover_format = CoverFormat::Jpeg;
          } else if (lower.indexOf("bmp") >= 0) {
            out.cover_format = CoverFormat::Bmp;
          } else {
            out.cover_format = CoverFormat::Unknown;
          }
        }

        f.seek(frame_start + frame_size);
        pos += frame_size;
      } else {
        f.seek(f.position() + frame_size);
        pos += frame_size;
      }
    } else {
      f.seek(f.position() + frame_size);
      pos += frame_size;
    }
  }

  f.close();
  return true;
}

static void add_unique(const char **arr, int &count, int max,
                       const char *value) {
  if (!value || value[0] == '\0') {
    return;
  }
  for (int i = 0; i < count; ++i) {
    if (same_str(arr[i], value)) {
      return;
    }
  }
  if (count < max) {
    arr[count++] = value;
  }
}

static void add_unique_album(AlbumInfo *arr, int &count, int max,
                             const char *name, const char *artist) {
  if (!name || name[0] == '\0') {
    return;
  }
  for (int i = 0; i < count; ++i) {
    if (same_str(arr[i].name, name) && same_str(arr[i].artist, artist)) {
      return;
    }
  }
  if (count < max) {
    arr[count].name = name;
    arr[count].artist = artist;
    ++count;
  }
}

static bool is_supported_audio(const String &path) {
  String lower = path;
  lower.toLowerCase();
  int dot = lower.lastIndexOf('.');
  if (dot < 0) {
    return false;
  }
  String ext = lower.substring(dot + 1);
  return (ext == "mp3" || ext == "wav");
}

static void scan_dir(Library &lib, fs::FS &fs, const String &dir,
                     uint8_t levels, int max_files, bool read_tags,
                     void (*tick)(), int &files_seen) {
  if (lib.track_count >= max_files) {
    return;
  }

  File root = fs.open(dir.c_str());
  if (!root || !root.isDirectory()) {
    return;
  }

  File file = root.openNextFile();
  while (file && lib.track_count < max_files) {
    if (file.isDirectory()) {
      if (levels > 0) {
        String sub = String(file.name());
        if (!sub.startsWith("/")) {
          sub = dir + String("/") + sub;
        }
        scan_dir(lib, fs, sub, levels - 1, max_files, read_tags, tick,
                 files_seen);
      }
    } else {
      String fname = String(file.name());
      if (!fname.startsWith("/")) {
        fname = dir + String("/") + fname;
      }
      ++files_seen;
      if (files_seen % 5 == 0) {
        if (tick) {
          tick();
        } else {
          yield();
        }
      }
      if (!is_supported_audio(fname)) {
        file = root.openNextFile();
        continue;
      }

      TrackInfo &track = lib.tracks[lib.track_count];
      track.path = lib.pool.store(fname);
      track.added_time = file.getLastWrite();

      TagInfo tags;
      if (read_tags) {
        read_id3_tags(fs, fname, tags);
      }

      String title = tags.title;
      String artist = tags.artist;
      String album = tags.album;
      String genre = tags.genre;
      String composer = tags.composer;

      if (title.length() == 0) {
        title = basename_no_ext(fname);
      }
      if (artist.length() == 0) {
        String guess = parent_dir(fname, 2);
        artist = guess.length() > 0 ? guess : kUnknownArtist;
      }
      if (album.length() == 0) {
        String guess = parent_dir(fname, 1);
        album = guess.length() > 0 ? guess : kUnknownAlbum;
      }
      if (genre.length() == 0) {
        genre = kUnknownGenre;
      }
      if (composer.length() == 0) {
        composer = kUnknownComposer;
      }

      track.title = lib.pool.store(
          empty_or(title.c_str(), basename_no_ext(fname).c_str()));
      track.artist = lib.pool.store(empty_or(artist.c_str(), kUnknownArtist));
      track.album = lib.pool.store(empty_or(album.c_str(), kUnknownAlbum));
      track.genre = lib.pool.store(empty_or(genre.c_str(), kUnknownGenre));
      track.composer =
          lib.pool.store(empty_or(composer.c_str(), kUnknownComposer));
      track.cover_pos = tags.cover_pos;
      track.cover_len = tags.cover_len;
      track.cover_format = tags.cover_format;

      add_unique(lib.artists, lib.artist_count, kMaxArtists, track.artist);
      add_unique_album(lib.albums, lib.album_count, kMaxAlbums, track.album,
                       track.artist);
      add_unique(lib.genres, lib.genre_count, kMaxGenres, track.genre);
      add_unique(lib.composers, lib.composer_count, kMaxComposers,
                 track.composer);

      lib.track_count++;
    }
    file = root.openNextFile();
  }
}

} // namespace

void StringPool::init(size_t cap) {
  if (data) {
    return;
  }
  capacity = cap;
  data = static_cast<char *>(
      heap_caps_malloc(cap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (data) {
    in_psram = true;
    return;
  }
  data = static_cast<char *>(malloc(cap));
  if (data) {
    in_psram = false;
  }
}

void StringPool::reset() { used = 0; }

const char *StringPool::store_cstr(const char *s) {
  if (!s || s[0] == '\0') {
    return "";
  }
  size_t len = strlen(s);
  size_t need = len + 1;
  if (!data || used + need > capacity) {
    char *dup = static_cast<char *>(malloc(need));
    if (!dup) {
      return "";
    }
    memcpy(dup, s, need);
    return dup;
  }
  char *out = data + used;
  memcpy(out, s, need);
  used += need;
  return out;
}

const char *StringPool::store(const String &s) {
  if (s.length() == 0) {
    return "";
  }
  return store_cstr(s.c_str());
}

void library_reset(Library &lib) {
  lib.pool.init(kStringPoolSize);
  lib.pool.reset();
  lib.track_count = 0;
  lib.artist_count = 0;
  lib.album_count = 0;
  lib.genre_count = 0;
  lib.composer_count = 0;
  lib.scanned = false;
}

bool library_scan(Library &lib, fs::FS &fs, const char *root_dir, uint8_t depth,
                  int max_files, bool read_tags, void (*tick)()) {
  library_reset(lib);
  String root = root_dir && root_dir[0] ? root_dir : "/";
  if (!root.startsWith("/")) {
    root = String("/") + root;
  }

  int limit = max_files;
  if (limit <= 0 || limit > kMaxTracks) {
    limit = kMaxTracks;
  }

  int files_seen = 0;
  scan_dir(lib, fs, root, depth, limit, read_tags, tick, files_seen);

  lib.scanned = true;
  return lib.track_count > 0;
}

int library_find_artist(const Library &lib, const String &name) {
  for (int i = 0; i < lib.artist_count; ++i) {
    if (name == lib.artists[i]) {
      return i;
    }
  }
  return -1;
}

int library_find_album(const Library &lib, const String &name,
                       const String &artist) {
  for (int i = 0; i < lib.album_count; ++i) {
    if (name == lib.albums[i].name && artist == lib.albums[i].artist) {
      return i;
    }
  }
  return -1;
}

int library_tracks_for_artist(const Library &lib, const String &artist,
                              int *out, int max) {
  int count = 0;
  for (int i = 0; i < lib.track_count && count < max; ++i) {
    if (artist == lib.tracks[i].artist) {
      out[count++] = i;
    }
  }
  return count;
}

int library_tracks_for_album(const Library &lib, const String &artist,
                             const String &album, int *out, int max) {
  int count = 0;
  for (int i = 0; i < lib.track_count && count < max; ++i) {
    if (album == lib.tracks[i].album &&
        (artist.length() == 0 || artist == lib.tracks[i].artist)) {
      out[count++] = i;
    }
  }
  return count;
}

int library_tracks_for_genre(const Library &lib, const String &genre, int *out,
                             int max) {
  int count = 0;
  for (int i = 0; i < lib.track_count && count < max; ++i) {
    if (genre == lib.tracks[i].genre) {
      out[count++] = i;
    }
  }
  return count;
}

int library_tracks_for_composer(const Library &lib, const String &composer,
                                int *out, int max) {
  int count = 0;
  for (int i = 0; i < lib.track_count && count < max; ++i) {
    if (composer == lib.tracks[i].composer) {
      out[count++] = i;
    }
  }
  return count;
}

int library_albums_for_artist(const Library &lib, const String &artist,
                              int *out, int max) {
  int count = 0;
  for (int i = 0; i < lib.album_count && count < max; ++i) {
    if (artist == lib.albums[i].artist) {
      out[count++] = i;
    }
  }
  return count;
}

} // namespace app
