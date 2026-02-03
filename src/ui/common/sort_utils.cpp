#include "ui/common/sort_utils.h"

#include <Arduino.h>

namespace lofi::ui::sort {
namespace {
char lower_char(char c) {
  if (c >= 'A' && c <= 'Z') {
    return static_cast<char>(c + ('a' - 'A'));
  }
  return c;
}
} // namespace

int compare_ci(const char *a, const char *b) {
  String sa = a ? a : "";
  String sb = b ? b : "";
  size_t la = sa.length();
  size_t lb = sb.length();
  size_t l = (la < lb) ? la : lb;
  for (size_t i = 0; i < l; ++i) {
    char ca = lower_char(sa[i]);
    char cb = lower_char(sb[i]);
    if (ca < cb) {
      return -1;
    }
    if (ca > cb) {
      return 1;
    }
  }
  if (la == lb) {
    return 0;
  }
  return (la < lb) ? -1 : 1;
}

void string_indices(const char *const *arr, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      if (compare_ci(arr[idx[j]], arr[idx[i]]) < 0) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}

void album_indices(const app::Library &lib, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      const app::AlbumInfo &a = lib.albums[idx[j]];
      const app::AlbumInfo &b = lib.albums[idx[i]];
      int c = compare_ci(a.name, b.name);
      if (c == 0) {
        c = compare_ci(a.artist, b.artist);
      }
      if (c < 0) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}

void track_indices_by_title(const app::Library &lib, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      const app::TrackInfo &a = lib.tracks[idx[j]];
      const app::TrackInfo &b = lib.tracks[idx[i]];
      int c = compare_ci(a.title, b.title);
      if (c == 0) {
        c = compare_ci(a.artist, b.artist);
      }
      if (c < 0) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}

void tracks_by_added(const app::Library &lib, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      const app::TrackInfo &a = lib.tracks[idx[j]];
      const app::TrackInfo &b = lib.tracks[idx[i]];
      if (a.added_time > b.added_time) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}

void tracks_by_play_count(const app::Library &lib, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      const app::TrackInfo &a = lib.tracks[idx[j]];
      const app::TrackInfo &b = lib.tracks[idx[i]];
      if (a.play_count > b.play_count) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}

void tracks_by_last_played(const app::Library &lib, int *idx, int count) {
  for (int i = 0; i < count - 1; ++i) {
    for (int j = i + 1; j < count; ++j) {
      const app::TrackInfo &a = lib.tracks[idx[j]];
      const app::TrackInfo &b = lib.tracks[idx[i]];
      if (a.last_played > b.last_played) {
        int tmp = idx[i];
        idx[i] = idx[j];
        idx[j] = tmp;
      }
    }
  }
}
} // namespace lofi::ui::sort
