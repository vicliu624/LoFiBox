#include "ui/screens/list_page/list_page_build.h"

#include "ui/screens/about/about_components.h"
#include "ui/screens/about/about_input.h"
#include "ui/screens/about/about_layout.h"
#include "ui/screens/about/about_styles.h"
#include "ui/screens/albums/albums_components.h"
#include "ui/screens/albums/albums_input.h"
#include "ui/screens/albums/albums_layout.h"
#include "ui/screens/albums/albums_styles.h"
#include "ui/screens/artists/artists_components.h"
#include "ui/screens/artists/artists_input.h"
#include "ui/screens/artists/artists_layout.h"
#include "ui/screens/artists/artists_styles.h"
#include "ui/screens/compilations/compilations_components.h"
#include "ui/screens/compilations/compilations_input.h"
#include "ui/screens/compilations/compilations_layout.h"
#include "ui/screens/compilations/compilations_styles.h"
#include "ui/screens/composers/composers_components.h"
#include "ui/screens/composers/composers_input.h"
#include "ui/screens/composers/composers_layout.h"
#include "ui/screens/composers/composers_styles.h"
#include "ui/screens/genres/genres_components.h"
#include "ui/screens/genres/genres_input.h"
#include "ui/screens/genres/genres_layout.h"
#include "ui/screens/genres/genres_styles.h"
#include "ui/screens/list_page/list_page_input.h"
#include "ui/screens/music/music_components.h"
#include "ui/screens/music/music_input.h"
#include "ui/screens/music/music_layout.h"
#include "ui/screens/music/music_styles.h"
#include "ui/screens/playlist_detail/playlist_detail_components.h"
#include "ui/screens/playlist_detail/playlist_detail_input.h"
#include "ui/screens/playlist_detail/playlist_detail_layout.h"
#include "ui/screens/playlist_detail/playlist_detail_styles.h"
#include "ui/screens/playlists/playlists_components.h"
#include "ui/screens/playlists/playlists_input.h"
#include "ui/screens/playlists/playlists_layout.h"
#include "ui/screens/playlists/playlists_styles.h"
#include "ui/screens/settings/settings_components.h"
#include "ui/screens/settings/settings_input.h"
#include "ui/screens/settings/settings_layout.h"
#include "ui/screens/settings/settings_styles.h"
#include "ui/screens/songs/songs_components.h"
#include "ui/screens/songs/songs_input.h"
#include "ui/screens/songs/songs_layout.h"
#include "ui/screens/songs/songs_styles.h"

namespace lofi::ui::screens::list_page {
namespace {
inline BuildApi
make_list_api(void (*init_styles)(), void (*apply_content)(lv_obj_t *),
              void (*apply_list)(lv_obj_t *), void (*apply_row)(lv_obj_t *),
              void (*apply_left)(lv_obj_t *), void (*apply_right)(lv_obj_t *),
              list_page::layout::ListLayout (*create_list)(lv_obj_t *),
              list_page::layout::ListRowLayout (*create_row)(lv_obj_t *),
              void (*attach_row)(UiScreen &, RowMeta &),
              void (*focus_first)(lv_group_t *, lv_obj_t *)) {
  BuildApi api{};
  api.init_styles = init_styles;
  api.apply_content = apply_content;
  api.apply_list = apply_list;
  api.apply_row = apply_row;
  api.apply_left = apply_left;
  api.apply_right = apply_right;
  api.create_list = create_list;
  api.create_row = create_row;
  api.attach_row = attach_row;
  api.focus_first = focus_first;
  return api;
}
} // namespace

BuildApi api_for(PageId id) {
  switch (id) {
  case PageId::Music:
    return make_list_api(screens::music::styles::init_once,
                         screens::music::styles::apply_content,
                         screens::music::styles::apply_list,
                         screens::music::styles::apply_list_row,
                         screens::music::styles::apply_list_label_left,
                         screens::music::styles::apply_list_label_right,
                         screens::music::layout::create_list,
                         screens::music::layout::create_list_row,
                         screens::music::input::attach_row,
                         screens::music::input::focus_first);
  case PageId::Artists:
    return make_list_api(screens::artists::styles::init_once,
                         screens::artists::styles::apply_content,
                         screens::artists::styles::apply_list,
                         screens::artists::styles::apply_list_row,
                         screens::artists::styles::apply_list_label_left,
                         screens::artists::styles::apply_list_label_right,
                         screens::artists::layout::create_list,
                         screens::artists::layout::create_list_row,
                         screens::artists::input::attach_row,
                         screens::artists::input::focus_first);
  case PageId::Albums:
    return make_list_api(screens::albums::styles::init_once,
                         screens::albums::styles::apply_content,
                         screens::albums::styles::apply_list,
                         screens::albums::styles::apply_list_row,
                         screens::albums::styles::apply_list_label_left,
                         screens::albums::styles::apply_list_label_right,
                         screens::albums::layout::create_list,
                         screens::albums::layout::create_list_row,
                         screens::albums::input::attach_row,
                         screens::albums::input::focus_first);
  case PageId::Songs:
    return make_list_api(screens::songs::styles::init_once,
                         screens::songs::styles::apply_content,
                         screens::songs::styles::apply_list,
                         screens::songs::styles::apply_list_row,
                         screens::songs::styles::apply_list_label_left,
                         screens::songs::styles::apply_list_label_right,
                         screens::songs::layout::create_list,
                         screens::songs::layout::create_list_row,
                         screens::songs::input::attach_row,
                         screens::songs::input::focus_first);
  case PageId::Genres:
    return make_list_api(screens::genres::styles::init_once,
                         screens::genres::styles::apply_content,
                         screens::genres::styles::apply_list,
                         screens::genres::styles::apply_list_row,
                         screens::genres::styles::apply_list_label_left,
                         screens::genres::styles::apply_list_label_right,
                         screens::genres::layout::create_list,
                         screens::genres::layout::create_list_row,
                         screens::genres::input::attach_row,
                         screens::genres::input::focus_first);
  case PageId::Composers:
    return make_list_api(screens::composers::styles::init_once,
                         screens::composers::styles::apply_content,
                         screens::composers::styles::apply_list,
                         screens::composers::styles::apply_list_row,
                         screens::composers::styles::apply_list_label_left,
                         screens::composers::styles::apply_list_label_right,
                         screens::composers::layout::create_list,
                         screens::composers::layout::create_list_row,
                         screens::composers::input::attach_row,
                         screens::composers::input::focus_first);
  case PageId::Compilations:
    return make_list_api(screens::compilations::styles::init_once,
                         screens::compilations::styles::apply_content,
                         screens::compilations::styles::apply_list,
                         screens::compilations::styles::apply_list_row,
                         screens::compilations::styles::apply_list_label_left,
                         screens::compilations::styles::apply_list_label_right,
                         screens::compilations::layout::create_list,
                         screens::compilations::layout::create_list_row,
                         screens::compilations::input::attach_row,
                         screens::compilations::input::focus_first);
  case PageId::Playlists:
    return make_list_api(screens::playlists::styles::init_once,
                         screens::playlists::styles::apply_content,
                         screens::playlists::styles::apply_list,
                         screens::playlists::styles::apply_list_row,
                         screens::playlists::styles::apply_list_label_left,
                         screens::playlists::styles::apply_list_label_right,
                         screens::playlists::layout::create_list,
                         screens::playlists::layout::create_list_row,
                         screens::playlists::input::attach_row,
                         screens::playlists::input::focus_first);
  case PageId::PlaylistDetail:
    return make_list_api(
        screens::playlist_detail::styles::init_once,
        screens::playlist_detail::styles::apply_content,
        screens::playlist_detail::styles::apply_list,
        screens::playlist_detail::styles::apply_list_row,
        screens::playlist_detail::styles::apply_list_label_left,
        screens::playlist_detail::styles::apply_list_label_right,
        screens::playlist_detail::layout::create_list,
        screens::playlist_detail::layout::create_list_row,
        screens::playlist_detail::input::attach_row,
        screens::playlist_detail::input::focus_first);
  case PageId::Settings:
    return make_list_api(screens::settings::styles::init_once,
                         screens::settings::styles::apply_content,
                         screens::settings::styles::apply_list,
                         screens::settings::styles::apply_list_row,
                         screens::settings::styles::apply_list_label_left,
                         screens::settings::styles::apply_list_label_right,
                         screens::settings::layout::create_list,
                         screens::settings::layout::create_list_row,
                         screens::settings::input::attach_row,
                         screens::settings::input::focus_first);
  case PageId::About:
    return make_list_api(screens::about::styles::init_once,
                         screens::about::styles::apply_content,
                         screens::about::styles::apply_list,
                         screens::about::styles::apply_list_row,
                         screens::about::styles::apply_list_label_left,
                         screens::about::styles::apply_list_label_right,
                         screens::about::layout::create_list,
                         screens::about::layout::create_list_row,
                         screens::about::input::attach_row,
                         screens::about::input::focus_first);
  default:
    return make_list_api(screens::music::styles::init_once,
                         screens::music::styles::apply_content,
                         screens::music::styles::apply_list,
                         screens::music::styles::apply_list_row,
                         screens::music::styles::apply_list_label_left,
                         screens::music::styles::apply_list_label_right,
                         screens::music::layout::create_list,
                         screens::music::layout::create_list_row,
                         screens::music::input::attach_row,
                         screens::music::input::focus_first);
  }
}

void populate_list(UiScreen &screen) {
  switch (screen.state.current) {
  case PageId::Music:
    screens::music::populate(screen);
    break;
  case PageId::Artists:
    screens::artists::populate(screen);
    break;
  case PageId::Albums:
    screens::albums::populate(screen);
    break;
  case PageId::Songs:
    screens::songs::populate(screen);
    break;
  case PageId::Genres:
    screens::genres::populate(screen);
    break;
  case PageId::Composers:
    screens::composers::populate(screen);
    break;
  case PageId::Compilations:
    screens::compilations::populate(screen);
    break;
  case PageId::Playlists:
    screens::playlists::populate(screen);
    break;
  case PageId::PlaylistDetail:
    screens::playlist_detail::populate(screen);
    break;
  case PageId::Settings:
    screens::settings::populate(screen);
    break;
  case PageId::About:
    screens::about::populate(screen);
    break;
  default:
    break;
  }
}

void build_list_page(UiScreen &screen, const BuildApi &api) {
  api.init_styles();
  api.apply_content(screen.view.root.content);
  screen.view.list = api.create_list(screen.view.root.content);
  api.apply_list(screen.view.list.list);
  const bool allow_scroll = (screen.state.current == PageId::About);
  const bool preserve_selection =
      (screen.state.last_list_page == screen.state.current);
  int selected = preserve_selection ? screen.state.list_selected : 0;
  int offset = preserve_selection ? screen.state.list_offset : 0;
  if (screen.view.list.list) {
    if (allow_scroll) {
      lv_obj_add_flag(screen.view.list.list, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_scroll_dir(screen.view.list.list, LV_DIR_VER);
    } else {
      lv_obj_clear_flag(screen.view.list.list, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_set_scroll_dir(screen.view.list.list, LV_DIR_NONE);
    }
  }
  if (screen.view.root.content) {
    lv_obj_update_layout(screen.view.root.content);
  }
  if (screen.view.list.list) {
    lv_obj_update_layout(screen.view.list.list);
  }

  lv_coord_t list_h = screen.view.list.list
                          ? lv_obj_get_height(screen.view.list.list)
                          : lv_obj_get_height(screen.view.root.content);
  if (list_h <= 0) {
    lv_coord_t full_h = lv_display_get_vertical_resolution(nullptr);
    lv_coord_t top_h = screens::common::layout::topbar_height();
    list_h = (full_h > top_h) ? (full_h - top_h) : full_h;
  }
  lv_coord_t row_h = list_page::layout::row_height();
  int visible = (row_h > 0) ? static_cast<int>(list_h / row_h) : 0;
  if (visible < 1) {
    visible = 1;
  }
  if (allow_scroll) {
    visible = screen.items_count;
  } else if (visible > screen.items_count) {
    visible = screen.items_count;
  }

  screen.row_count = 0;
  for (int i = 0; i < visible; ++i) {
    list_page::layout::ListRowLayout row_layout =
        api.create_row(screen.view.list.list);
    api.apply_row(row_layout.row);
    api.apply_left(row_layout.left_label);
    api.apply_right(row_layout.right_label);

    RowMeta &meta = screen.rows[screen.row_count++];
    meta.row = row_layout.row;
    meta.icon = row_layout.icon;
    meta.left_label = row_layout.left_label;
    meta.right_label = row_layout.right_label;
    api.attach_row(screen, meta);
  }

  if (screen.items_count <= 0) {
    selected = 0;
    offset = 0;
  } else {
    if (selected < 0 || selected >= screen.items_count) {
      selected = 0;
    }
    if (allow_scroll) {
      offset = 0;
    } else {
      int max_offset = screen.items_count - visible;
      if (max_offset < 0) {
        max_offset = 0;
      }
      if (offset < 0) {
        offset = 0;
      }
      if (offset > max_offset) {
        offset = max_offset;
      }
      if (selected < offset) {
        offset = selected;
      } else if (selected >= offset + visible) {
        offset = selected - visible + 1;
      }
      if (offset < 0) {
        offset = 0;
      }
      if (offset > max_offset) {
        offset = max_offset;
      }
    }
  }
  screen.state.list_offset = offset;
  screen.state.list_selected = selected;
  screen.state.last_list_page = screen.state.current;
  list_page::input::refresh_rows(screen);
}
} // namespace lofi::ui::screens::list_page
