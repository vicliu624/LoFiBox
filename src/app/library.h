#pragma once

#include <Arduino.h>
#include <FS.h>

namespace app
{

constexpr int kMaxTracks = 512;
constexpr int kMaxArtists = 128;
constexpr int kMaxAlbums = 256;
constexpr int kMaxGenres = 64;
constexpr int kMaxComposers = 64;
constexpr int kMaxPlaylistTracks = 256;

struct TrackInfo
{
    String path;
    String title;
    String artist;
    String album;
    String genre;
    String composer;
    uint32_t duration_sec = 0;
    uint32_t added_time = 0;
    uint32_t play_count = 0;
    uint32_t last_played = 0;
};

struct AlbumInfo
{
    String name;
    String artist;
};

struct Library
{
    TrackInfo tracks[kMaxTracks];
    int track_count = 0;

    String artists[kMaxArtists];
    int artist_count = 0;

    AlbumInfo albums[kMaxAlbums];
    int album_count = 0;

    String genres[kMaxGenres];
    int genre_count = 0;

    String composers[kMaxComposers];
    int composer_count = 0;
};

void library_reset(Library& lib);
bool library_scan(Library& lib, fs::FS& fs, const char* root_dir, uint8_t depth);

int library_find_artist(const Library& lib, const String& name);
int library_find_album(const Library& lib, const String& name, const String& artist);

int library_tracks_for_artist(const Library& lib, const String& artist, int* out, int max);
int library_tracks_for_album(const Library& lib, const String& artist, const String& album, int* out, int max);
int library_tracks_for_genre(const Library& lib, const String& genre, int* out, int max);
int library_tracks_for_composer(const Library& lib, const String& composer, int* out, int max);

int library_albums_for_artist(const Library& lib, const String& artist, int* out, int max);

} // namespace app