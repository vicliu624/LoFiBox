#include "ui/screens/songs/songs_components.h"

namespace lofi::ui::screens::songs
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    if (!screen.library || screen.library->track_count == 0) {
        components::add_item(screen, "No Songs", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    int idx[app::kMaxTracks];
    int count = 0;

    switch (screen.state.song_context) {
    case SongContext::Artist:
        count = app::library_tracks_for_artist(*screen.library, screen.state.selected_artist, idx, app::kMaxTracks);
        break;
    case SongContext::Album: {
        String artist = screen.state.selected_album_artist;
        count = app::library_tracks_for_album(*screen.library, artist, screen.state.selected_album, idx, app::kMaxTracks);
        break;
    }
    case SongContext::Genre:
        count = app::library_tracks_for_genre(*screen.library, screen.state.selected_genre, idx, app::kMaxTracks);
        break;
    case SongContext::Composer:
        count = app::library_tracks_for_composer(*screen.library, screen.state.selected_composer, idx, app::kMaxTracks);
        break;
    case SongContext::All:
    default:
        for (int i = 0; i < screen.library->track_count; ++i) {
            idx[count++] = i;
        }
        break;
    }

    if (count == 0) {
        components::add_item(screen, "No Songs", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    components::sort_track_indices_by_title(*screen.library, idx, count);

    for (int i = 0; i < count; ++i) {
        const app::TrackInfo& track = screen.library->tracks[idx[i]];
        components::add_item(screen, track.title, "", UiIntentKind::PlayTrack, PageId::NowPlaying, idx[i]);
    }
}

} // namespace lofi::ui::screens::songs
