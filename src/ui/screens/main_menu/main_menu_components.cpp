#include "ui/screens/main_menu/main_menu_components.h"

#include "ui/assets/assets.h"

namespace lofi::ui::screens::main_menu
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    components::add_item(screen, "Music", nullptr, UiIntentKind::Navigate, PageId::Music, 0, 0, &Music);
    components::add_item(screen, "Playlists", nullptr, UiIntentKind::Navigate, PageId::Playlists, 0, 0, &Playlists);
    components::add_item(screen, "Now Playing", nullptr, UiIntentKind::Navigate, PageId::NowPlaying, 0, 0, &NowPlaying);
    components::add_item(screen, "Settings", nullptr, UiIntentKind::Navigate, PageId::Settings, 0, 0, &Settings);
}

} // namespace lofi::ui::screens::main_menu
