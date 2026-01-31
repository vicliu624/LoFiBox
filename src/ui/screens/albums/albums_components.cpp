#include "ui/screens/albums/albums_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::albums
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    if (!screen.library || screen.library->album_count == 0) {
        components::add_item(screen, "No Music", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    int idx[app::kMaxAlbums];
    int count = 0;
    if (screen.state.album_filter == AlbumFilter::Artist && screen.state.selected_artist.length() > 0) {
        count = app::library_albums_for_artist(*screen.library, screen.state.selected_artist, idx, app::kMaxAlbums);
    } else {
        for (int i = 0; i < screen.library->album_count; ++i) {
            idx[count++] = i;
        }
    }
    sort::album_indices(*screen.library, idx, count);

    for (int i = 0; i < count; ++i) {
        const app::AlbumInfo& album = screen.library->albums[idx[i]];
        components::add_item(screen, album.name, album.artist, UiIntentKind::OpenAlbum, PageId::Songs, idx[i]);
    }
    if (count == 0) {
        components::add_item(screen, "No Albums", nullptr, UiIntentKind::None, PageId::None);
    }
}

} // namespace lofi::ui::screens::albums
