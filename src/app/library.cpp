#include "app/library.h"

#include <SD.h>

namespace app
{
namespace
{
const char* kUnknownArtist = "Unknown Artist";
const char* kUnknownAlbum = "Unknown Album";
const char* kUnknownGenre = "Unknown Genre";
const char* kUnknownComposer = "Unknown Composer";

struct TagInfo
{
    String title;
    String artist;
    String album;
    String genre;
    String composer;
};

static uint32_t read_u32_be(const uint8_t* data)
{
    return (static_cast<uint32_t>(data[0]) << 24) |
           (static_cast<uint32_t>(data[1]) << 16) |
           (static_cast<uint32_t>(data[2]) << 8) |
           static_cast<uint32_t>(data[3]);
}

static uint32_t read_syncsafe(const uint8_t* data)
{
    return (static_cast<uint32_t>(data[0] & 0x7F) << 21) |
           (static_cast<uint32_t>(data[1] & 0x7F) << 14) |
           (static_cast<uint32_t>(data[2] & 0x7F) << 7) |
           static_cast<uint32_t>(data[3] & 0x7F);
}

static String basename_no_ext(const String& path)
{
    int slash = path.lastIndexOf('/');
    String name = (slash >= 0) ? path.substring(slash + 1) : path;
    int dot = name.lastIndexOf('.');
    if (dot > 0) {
        name = name.substring(0, dot);
    }
    name.trim();
    return name;
}

static String parent_dir(const String& path, int levels_up)
{
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

static String decode_text_frame(uint8_t encoding, const uint8_t* data, size_t len)
{
    if (len == 0) {
        return String();
    }

    if (encoding == 0 || encoding == 3) {
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

    // UTF-16: best-effort, strip BOM and take low bytes
    size_t start = 0;
    if (len >= 2) {
        if ((data[0] == 0xFF && data[1] == 0xFE) || (data[0] == 0xFE && data[1] == 0xFF)) {
            start = 2;
        }
    }
    String out;
    out.reserve(len / 2 + 1);
    for (size_t i = start; i + 1 < len; i += 2) {
        uint8_t b = data[i];
        if (b == 0x00 && data[i + 1] == 0x00) {
            break;
        }
        if (b == 0x00) {
            b = data[i + 1];
        }
        if (b == 0x00) {
            continue;
        }
        out += static_cast<char>(b);
    }
    out.trim();
    return out;
}

static void assign_if_empty(String& target, const String& value)
{
    if (target.length() == 0 && value.length() > 0) {
        target = value;
    }
}

static bool read_id3_tags(fs::FS& fs, const String& path, TagInfo& out)
{
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

        uint32_t frame_size = (version == 4) ? read_syncsafe(&frame_header[4]) : read_u32_be(&frame_header[4]);
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
        } else {
            f.seek(f.position() + frame_size);
            pos += frame_size;
        }
    }

    f.close();
    return true;
}

static void add_unique(String* arr, int& count, int max, const String& value)
{
    if (value.length() == 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        if (arr[i] == value) {
            return;
        }
    }
    if (count < max) {
        arr[count++] = value;
    }
}

static void add_unique_album(AlbumInfo* arr, int& count, int max, const String& name, const String& artist)
{
    if (name.length() == 0) {
        return;
    }
    for (int i = 0; i < count; ++i) {
        if (arr[i].name == name && arr[i].artist == artist) {
            return;
        }
    }
    if (count < max) {
        arr[count].name = name;
        arr[count].artist = artist;
        ++count;
    }
}

static bool is_supported_audio(const String& path)
{
    String lower = path;
    lower.toLowerCase();
    int dot = lower.lastIndexOf('.');
    if (dot < 0) {
        return false;
    }
    String ext = lower.substring(dot + 1);
    return (ext == "mp3" || ext == "wav");
}

static void scan_dir(Library& lib, fs::FS& fs, const String& dir, uint8_t levels, int max_files, bool read_tags,
                     void (*tick)(), int& files_seen)
{
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
                scan_dir(lib, fs, sub, levels - 1, max_files, read_tags, tick, files_seen);
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

            TrackInfo& track = lib.tracks[lib.track_count];
            track.path = fname;
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

            track.title = title;
            track.artist = artist;
            track.album = album;
            track.genre = genre;
            track.composer = composer;

            add_unique(lib.artists, lib.artist_count, kMaxArtists, artist);
            add_unique_album(lib.albums, lib.album_count, kMaxAlbums, album, artist);
            add_unique(lib.genres, lib.genre_count, kMaxGenres, genre);
            add_unique(lib.composers, lib.composer_count, kMaxComposers, composer);

            lib.track_count++;
        }
        file = root.openNextFile();
    }
}

} // namespace

void library_reset(Library& lib)
{
    lib.track_count = 0;
    lib.artist_count = 0;
    lib.album_count = 0;
    lib.genre_count = 0;
    lib.composer_count = 0;
    lib.scanned = false;
}

bool library_scan(Library& lib, fs::FS& fs, const char* root_dir, uint8_t depth, int max_files, bool read_tags,
                  void (*tick)())
{
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

int library_find_artist(const Library& lib, const String& name)
{
    for (int i = 0; i < lib.artist_count; ++i) {
        if (lib.artists[i] == name) {
            return i;
        }
    }
    return -1;
}

int library_find_album(const Library& lib, const String& name, const String& artist)
{
    for (int i = 0; i < lib.album_count; ++i) {
        if (lib.albums[i].name == name && lib.albums[i].artist == artist) {
            return i;
        }
    }
    return -1;
}

int library_tracks_for_artist(const Library& lib, const String& artist, int* out, int max)
{
    int count = 0;
    for (int i = 0; i < lib.track_count && count < max; ++i) {
        if (lib.tracks[i].artist == artist) {
            out[count++] = i;
        }
    }
    return count;
}

int library_tracks_for_album(const Library& lib, const String& artist, const String& album, int* out, int max)
{
    int count = 0;
    for (int i = 0; i < lib.track_count && count < max; ++i) {
        if (lib.tracks[i].album == album && (artist.length() == 0 || lib.tracks[i].artist == artist)) {
            out[count++] = i;
        }
    }
    return count;
}

int library_tracks_for_genre(const Library& lib, const String& genre, int* out, int max)
{
    int count = 0;
    for (int i = 0; i < lib.track_count && count < max; ++i) {
        if (lib.tracks[i].genre == genre) {
            out[count++] = i;
        }
    }
    return count;
}

int library_tracks_for_composer(const Library& lib, const String& composer, int* out, int max)
{
    int count = 0;
    for (int i = 0; i < lib.track_count && count < max; ++i) {
        if (lib.tracks[i].composer == composer) {
            out[count++] = i;
        }
    }
    return count;
}

int library_albums_for_artist(const Library& lib, const String& artist, int* out, int max)
{
    int count = 0;
    for (int i = 0; i < lib.album_count && count < max; ++i) {
        if (lib.albums[i].artist == artist) {
            out[count++] = i;
        }
    }
    return count;
}

} // namespace app
