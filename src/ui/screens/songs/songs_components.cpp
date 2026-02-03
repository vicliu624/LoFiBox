#include "ui/screens/songs/songs_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::songs {
void populate(UiScreen &screen) {
  components::reset_items(screen);
  if (!screen.library || screen.library->track_count == 0) {
    components::add_item(screen, "No Songs", nullptr, UiIntentKind::None,
                         PageId::None);
    return;
  }

  int idx[app::kMaxTracks];
  int count = 0;

  switch (screen.state.song_context) {
  case SongContext::Artist:
    count = app::library_tracks_for_artist(
        *screen.library, screen.state.selected_artist, idx, app::kMaxTracks);
    break;
  case SongContext::Album: {
    String artist = screen.state.selected_album_artist;
    count = app::library_tracks_for_album(*screen.library, artist,
                                          screen.state.selected_album, idx,
                                          app::kMaxTracks);
    break;
  }
  case SongContext::Genre:
    count = app::library_tracks_for_genre(
        *screen.library, screen.state.selected_genre, idx, app::kMaxTracks);
    break;
  case SongContext::Composer:
    count = app::library_tracks_for_composer(
        *screen.library, screen.state.selected_composer, idx, app::kMaxTracks);
    break;
  case SongContext::All:
  default:
    for (int i = 0; i < screen.library->track_count; ++i) {
      idx[count++] = i;
    }
    break;
  }

  if (count == 0) {
    components::add_item(screen, "No Songs", nullptr, UiIntentKind::None,
                         PageId::None);
    return;
  }

  sort::track_indices_by_title(*screen.library, idx, count);

  bool has_zero = false;
  for (int i = 0; i < count; ++i) {
    if (idx[i] == 0) {
      has_zero = true;
      break;
    }
  }
  Serial.printf("[SONGS] context=%d count=%d has_idx0=%d first=\"%s\"\n",
                static_cast<int>(screen.state.song_context), count,
                has_zero ? 1 : 0,
                (count > 0 && screen.library->tracks[idx[0]].title)
                    ? screen.library->tracks[idx[0]].title
                    : "");

  for (int i = 0; i < count; ++i) {
    const app::TrackInfo &track = screen.library->tracks[idx[i]];
    components::add_item(screen, track.title, "", UiIntentKind::PlayTrack,
                         PageId::NowPlaying, idx[i]);
  }
}

} // namespace lofi::ui::screens::songs
