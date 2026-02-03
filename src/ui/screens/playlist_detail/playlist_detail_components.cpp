#include "ui/screens/playlist_detail/playlist_detail_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::playlist_detail {
namespace {
int build_playlist_tracks(UiScreen &screen) {
  screen.playlist_count = 0;
  if (!screen.library || screen.library->track_count == 0) {
    return 0;
  }

  if (screen.state.current_playlist == 0) {
    for (int i = 0; i < screen.on_the_go_count &&
                    screen.playlist_count < app::kMaxPlaylistTracks;
         ++i) {
      screen.playlist_tracks[screen.playlist_count++] = screen.on_the_go[i];
    }
    return screen.playlist_count;
  }

  for (int i = 0; i < screen.library->track_count &&
                  screen.playlist_count < app::kMaxPlaylistTracks;
       ++i) {
    screen.playlist_tracks[screen.playlist_count++] = i;
  }

  switch (screen.state.current_playlist) {
  case 1:
    sort::tracks_by_added(*screen.library, screen.playlist_tracks,
                          screen.playlist_count);
    break;
  case 2:
    sort::tracks_by_play_count(*screen.library, screen.playlist_tracks,
                               screen.playlist_count);
    break;
  case 3:
    sort::tracks_by_last_played(*screen.library, screen.playlist_tracks,
                                screen.playlist_count);
    break;
  default:
    break;
  }
  return screen.playlist_count;
}
} // namespace

void populate(UiScreen &screen) {
  components::reset_items(screen);
  if (build_playlist_tracks(screen) == 0) {
    components::add_item(screen, "Empty", nullptr, UiIntentKind::None,
                         PageId::None);
    return;
  }

  for (int i = 0; i < screen.playlist_count; ++i) {
    int idx = screen.playlist_tracks[i];
    if (idx < 0 || idx >= screen.library->track_count) {
      continue;
    }
    const app::TrackInfo &track = screen.library->tracks[idx];
    components::add_item(screen, track.title, track.artist,
                         UiIntentKind::PlayTrack, PageId::NowPlaying, idx);
  }
}

} // namespace lofi::ui::screens::playlist_detail
