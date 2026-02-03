#include "ui/screens/playlists/playlists_components.h"

#include "ui/assets/assets.h"

namespace lofi::ui::screens::playlists {
void populate(UiScreen &screen) {
  components::reset_items(screen);
  components::add_item(screen, "On-The-Go", nullptr, UiIntentKind::OpenPlaylist,
                       PageId::PlaylistDetail, 0, 0, &Playlists);
  components::add_item(screen, "Recently Added", nullptr,
                       UiIntentKind::OpenPlaylist, PageId::PlaylistDetail, 1, 0,
                       &Playlists);
  components::add_item(screen, "Most Played", nullptr,
                       UiIntentKind::OpenPlaylist, PageId::PlaylistDetail, 2, 0,
                       &Playlists);
  components::add_item(screen, "Recently Played", nullptr,
                       UiIntentKind::OpenPlaylist, PageId::PlaylistDetail, 3, 0,
                       &Playlists);
}

} // namespace lofi::ui::screens::playlists
