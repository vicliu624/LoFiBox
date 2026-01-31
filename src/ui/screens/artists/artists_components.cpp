#include "ui/screens/artists/artists_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::artists
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    if (!screen.library || screen.library->artist_count == 0) {
        components::add_item(screen, "No Music", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    int idx[app::kMaxArtists];
    for (int i = 0; i < screen.library->artist_count; ++i) {
        idx[i] = i;
    }
    sort::string_indices(screen.library->artists, idx, screen.library->artist_count);

    for (int i = 0; i < screen.library->artist_count; ++i) {
        components::add_item(screen, screen.library->artists[idx[i]], "", UiIntentKind::OpenArtist, PageId::Albums, idx[i]);
    }
}

} // namespace lofi::ui::screens::artists
