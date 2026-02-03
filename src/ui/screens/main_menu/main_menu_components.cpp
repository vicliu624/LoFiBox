#include "ui/screens/main_menu/main_menu_components.h"

#include "ui/assets/assets.h"

namespace lofi::ui::screens::main_menu {
void populate(UiScreen &screen) {
  components::reset_items(screen);
  components::add_item(screen, "Music", nullptr, UiIntentKind::OpenSongsAll,
                       PageId::Songs, 0, 0, &Music);
  components::add_item(screen, "Library", nullptr, UiIntentKind::Navigate,
                       PageId::Music, 0, 0, &Library);
  components::add_item(screen, "Playlists", nullptr, UiIntentKind::Navigate,
                       PageId::Playlists, 0, 0, &Playlists);
  components::add_item(screen, "Now Playing", nullptr, UiIntentKind::Navigate,
                       PageId::NowPlaying, 0, 0, &NowPlaying);
  components::add_item(screen, "Equalizer", nullptr, UiIntentKind::OpenEq,
                       PageId::Eq, 0, 0, &Equalizer);
  components::add_item(screen, "Settings", nullptr, UiIntentKind::Navigate,
                       PageId::Settings, 0, 0, &Settings);
  components::add_item(screen, "About", nullptr, UiIntentKind::OpenAbout,
                       PageId::About, 0, 0, &About);
}

} // namespace lofi::ui::screens::main_menu
