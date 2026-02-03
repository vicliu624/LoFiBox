#include "ui/screens/compilations/compilations_components.h"
#include "ui/common/sort_utils.h"
#include <cstring>

namespace lofi::ui::screens::compilations {
void populate(UiScreen &screen) {
  components::reset_items(screen);
  if (!screen.library || screen.library->album_count == 0) {
    components::add_item(screen, "No Music", nullptr, UiIntentKind::None,
                         PageId::None);
    return;
  }

  bool is_compilation[app::kMaxAlbums] = {};
  for (int i = 0; i < screen.library->album_count; ++i) {
    for (int j = i + 1; j < screen.library->album_count; ++j) {
      if (strcmp(screen.library->albums[i].name,
                 screen.library->albums[j].name) == 0 &&
          strcmp(screen.library->albums[i].artist,
                 screen.library->albums[j].artist) != 0) {
        is_compilation[i] = true;
        is_compilation[j] = true;
      }
    }
  }

  const char *names[app::kMaxAlbums] = {};
  int name_count = 0;
  for (int i = 0; i < screen.library->album_count; ++i) {
    if (!is_compilation[i]) {
      continue;
    }
    bool exists = false;
    for (int j = 0; j < name_count; ++j) {
      if (strcmp(names[j], screen.library->albums[i].name) == 0) {
        exists = true;
        break;
      }
    }
    if (!exists && name_count < app::kMaxAlbums) {
      names[name_count++] = screen.library->albums[i].name;
    }
  }

  if (name_count == 0) {
    components::add_item(screen, "No Compilations", nullptr, UiIntentKind::None,
                         PageId::None);
    return;
  }

  int idx[app::kMaxAlbums];
  for (int i = 0; i < name_count; ++i) {
    idx[i] = i;
  }
  sort::string_indices(names, idx, name_count);
  for (int i = 0; i < name_count; ++i) {
    components::add_item(screen, names[idx[i]], "", UiIntentKind::OpenAlbum,
                         PageId::Songs);
  }
}

} // namespace lofi::ui::screens::compilations
