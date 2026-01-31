#include "ui/screens/genres/genres_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::genres
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    if (!screen.library || screen.library->genre_count == 0) {
        components::add_item(screen, "No Genres", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    int idx[app::kMaxGenres];
    for (int i = 0; i < screen.library->genre_count; ++i) {
        idx[i] = i;
    }
    sort::string_indices(screen.library->genres, idx, screen.library->genre_count);

    for (int i = 0; i < screen.library->genre_count; ++i) {
        components::add_item(screen, screen.library->genres[idx[i]], "", UiIntentKind::OpenGenre, PageId::Songs, idx[i]);
    }
}

} // namespace lofi::ui::screens::genres
