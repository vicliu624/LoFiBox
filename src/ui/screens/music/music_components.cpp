#include "ui/screens/music/music_components.h"

#include "ui/assets/assets.h"

namespace lofi::ui::screens::music
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    components::add_item(screen, "Artists", nullptr, UiIntentKind::Navigate, PageId::Artists, 0, 0, nullptr);
    components::add_item(screen, "Albums", nullptr, UiIntentKind::OpenAlbumsAll, PageId::Albums, 0, 0, nullptr);
    components::add_item(screen, "Songs", nullptr, UiIntentKind::OpenSongsAll, PageId::Songs, 0, 0, nullptr);
    components::add_item(screen, "Genres", nullptr, UiIntentKind::Navigate, PageId::Genres, 0, 0, nullptr);
    components::add_item(screen, "Composers", nullptr, UiIntentKind::Navigate, PageId::Composers, 0, 0, nullptr);
    components::add_item(screen, "Compilations", nullptr, UiIntentKind::Navigate, PageId::Compilations, 0, 0, nullptr);
}

} // namespace lofi::ui::screens::music
