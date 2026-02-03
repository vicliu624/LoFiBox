#include "ui/lofibox/lofibox_ui.h"

#include <Arduino.h>
#include <SD.h>

#include "ui/fonts/fonts.h"
#include "ui/lofibox/lofibox_components.h"
#include "ui/lofibox/lofibox_ui_internal.h"

namespace lofi::ui {
namespace {
UiScreen s_screen;

void timer_delete(lv_timer_t *timer) {
#if defined(LVGL_VERSION_MAJOR) && (LVGL_VERSION_MAJOR >= 9)
  lv_timer_delete(timer);
#else
  lv_timer_del(timer);
#endif
}

void on_root_delete(lv_event_t *e) {
  auto *screen = static_cast<UiScreen *>(lv_event_get_user_data(e));
  if (!screen) {
    return;
  }

  screen->alive = false;
  screen->has_pending_intent = false;
  screen->timers.clear_all();
  screen->view = {};
  screen->delete_prompt_active = false;
  screen->delete_track_index = -1;
  screen->delete_overlay = nullptr;
  screen->delete_label = nullptr;

  if (screen->group) {
    lv_group_del(screen->group);
    screen->group = nullptr;
  }
}

void attach_delete_hook(UiScreen &screen) {
  if (!screen.view.root.root) {
    return;
  }
  lv_obj_add_event_cb(screen.view.root.root, on_root_delete, LV_EVENT_DELETE,
                      &screen);
}

void battery_timer_cb(lv_timer_t *timer) {
  auto *screen =
      static_cast<UiScreen *>(timer ? lv_timer_get_user_data(timer) : nullptr);
  if (!screen_alive(screen)) {
    return;
  }
  components::update_topbar(*screen);
}

void now_playing_timer_cb(lv_timer_t *timer) {
  auto *screen =
      static_cast<UiScreen *>(timer ? lv_timer_get_user_data(timer) : nullptr);
  if (!screen_alive(screen)) {
    return;
  }
  components::update_now_playing(*screen);
}

void start_timers(UiScreen &screen) {
  screen.timers.reset();
  screen.timers.create(TimerDomain::ScreenGeneral, battery_timer_cb, 60000,
                       &screen);
  screen.timers.create(TimerDomain::NowPlaying, now_playing_timer_cb, 500,
                       &screen);
}

void destroy_view(UiScreen &screen) {
  if (screen.view.root.root) {
    lv_obj_del(screen.view.root.root);
  }
}

void build_view(UiScreen &screen) {
  screen.alive = true;
  components::build_page(screen);
  attach_delete_hook(screen);
  start_timers(screen);
}

void navigate_to(UiScreen &screen, PageId id, bool push) {
  if (push && screen.state.current != PageId::None &&
      screen.state.depth < static_cast<int>(sizeof(screen.state.stack) /
                                            sizeof(screen.state.stack[0]))) {
    screen.state.stack[screen.state.depth++] = screen.state.current;
  }

  screen.state.current = id;
  destroy_view(screen);
  build_view(screen);
}

void navigate_back(UiScreen &screen) {
  if (screen.state.depth <= 0) {
    return;
  }
  PageId prev = screen.state.stack[--screen.state.depth];
  screen.state.current = prev;
  destroy_view(screen);
  build_view(screen);
}

void rebuild_current(UiScreen &screen) {
  destroy_view(screen);
  build_view(screen);
}

int resolve_delete_track(UiScreen &screen) {
  if (!screen.library) {
    return -1;
  }
  int idx = -1;
  if (screen.state.list_selected >= 0 &&
      screen.state.list_selected < screen.items_count) {
    ListItem &item = screen.items[screen.state.list_selected];
    if (item.action == UiIntentKind::PlayTrack) {
      idx = item.value;
    }
  }
  if (idx < 0 && screen.player) {
    idx = screen.player->current_index;
  }
  if (idx < 0 || idx >= screen.library->track_count) {
    return -1;
  }
  return idx;
}

void hide_delete_prompt(UiScreen &screen) {
  if (screen.delete_overlay) {
    lv_obj_del(screen.delete_overlay);
  }
  screen.delete_prompt_active = false;
  screen.delete_track_index = -1;
  screen.delete_overlay = nullptr;
  screen.delete_label = nullptr;
}

void show_delete_prompt(UiScreen &screen) {
  if (screen.delete_prompt_active) {
    return;
  }
  int idx = resolve_delete_track(screen);
  if (idx < 0) {
    return;
  }

  lv_obj_t *parent =
      screen.view.root.root ? screen.view.root.root : lv_screen_active();
  lv_obj_t *overlay = lv_obj_create(parent);
  lv_obj_remove_style_all(overlay);
  lv_obj_set_size(overlay, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(overlay, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(overlay, LV_OPA_70, 0);

  lv_obj_t *panel = lv_obj_create(overlay);
  lv_obj_set_size(panel, LV_PCT(80), LV_SIZE_CONTENT);
  lv_obj_center(panel);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x141414), 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x3a3a3a), 0);
  lv_obj_set_style_border_width(panel, 1, 0);
  lv_obj_set_style_pad_all(panel, 12, 0);

  lv_obj_t *label = lv_label_create(panel);
  const char *title = screen.library->tracks[idx].title;
  String text = "Delete this song?\n";
  if (title && title[0] != '\0') {
    text += "\"";
    text += title;
    text += "\"\n";
  }
  text += "Y=Confirm  C=Cancel";
  lv_label_set_text(label, text.c_str());
  lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(label, LV_PCT(100));

  screen.delete_prompt_active = true;
  screen.delete_track_index = idx;
  screen.delete_overlay = overlay;
  screen.delete_label = label;
}

void rescan_library(UiScreen &screen) {
  if (!screen.library) {
    return;
  }

  String keep_path;
  if (screen.player && screen.player->current_index >= 0 &&
      screen.player->current_index < screen.library->track_count) {
    const char *path =
        screen.library->tracks[screen.player->current_index].path;
    if (path) {
      keep_path = path;
    }
  }

  app::library_reset(*screen.library);
  app::library_scan(*screen.library, SD, "/music", 8, app::kMaxTracks, true,
                    nullptr);

  if (screen.player) {
    int new_index = -1;
    if (keep_path.length() > 0) {
      for (int i = 0; i < screen.library->track_count; ++i) {
        const char *path = screen.library->tracks[i].path;
        if (path && keep_path == path) {
          new_index = i;
          break;
        }
      }
    }
    screen.player->current_index = new_index;
    if (new_index < 0) {
      screen.player->is_playing = false;
      screen.player->paused = false;
    }
  }

  screen.state.list_offset = 0;
  screen.state.list_selected = 0;
  screen.state.last_track_index = -2;
  screen.state.last_meta_version = 0;
  screen.state.last_cover_version = 0;
}

void perform_delete(UiScreen &screen) {
  int idx = screen.delete_track_index;
  hide_delete_prompt(screen);
  if (!screen.library || idx < 0 || idx >= screen.library->track_count) {
    return;
  }

  bool deleting_current =
      (screen.player && screen.player->current_index == idx);
  if (deleting_current && screen.player) {
    app::player_stop(*screen.player);
  }

  const char *path = screen.library->tracks[idx].path;
  if (path && SD.exists(path)) {
    SD.remove(path);
  }

  rescan_library(screen);
  rebuild_current(screen);
}
} // namespace

void TimerBag::reset() { clear_all(); }

lv_timer_t *TimerBag::create(TimerDomain domain, lv_timer_cb_t cb,
                             uint32_t period, void *user_data) {
  if (count >= kMaxTimers) {
    return nullptr;
  }

  lv_timer_t *timer = lv_timer_create(cb, period, user_data);
  if (!timer) {
    return nullptr;
  }

  entries[count].timer = timer;
  entries[count].domain = domain;
  ++count;
  return timer;
}

void TimerBag::clear_domain(TimerDomain domain) {
  for (int i = 0; i < count;) {
    if (entries[i].timer && entries[i].domain == domain) {
      timer_delete(entries[i].timer);
      for (int j = i + 1; j < count; ++j) {
        entries[j - 1] = entries[j];
      }
      --count;
      continue;
    }
    ++i;
  }
}

void TimerBag::clear_all() {
  for (int i = 0; i < count; ++i) {
    if (entries[i].timer) {
      timer_delete(entries[i].timer);
    }
  }
  count = 0;
}

void request_intent(UiScreen *screen, const UiIntent &intent) {
  if (!screen_alive(screen)) {
    return;
  }
  screen->pending_intent = intent;
  screen->has_pending_intent = true;
}

bool screen_alive(const UiScreen *screen) { return screen && screen->alive; }

void init(app::Library *library, app::PlayerState *player) {
  init_font_fallbacks();
  s_screen.alive = false;
  s_screen.has_pending_intent = false;
  s_screen.pending_intent = {};
  s_screen.timers.clear_all();
  s_screen.view = {};
  s_screen.items_count = 0;
  s_screen.row_count = 0;
  s_screen.on_the_go_count = 0;
  s_screen.playlist_count = 0;
  if (s_screen.group) {
    lv_group_del(s_screen.group);
    s_screen.group = nullptr;
  }
  s_screen.library = library;
  s_screen.player = player;
  s_screen.state = {};
  s_screen.delete_prompt_active = false;
  s_screen.delete_track_index = -1;
  s_screen.delete_overlay = nullptr;
  s_screen.delete_label = nullptr;
  s_screen.state.current = PageId::MainMenu;
  s_screen.state.song_context = SongContext::All;
  s_screen.state.album_filter = AlbumFilter::All;
  s_screen.state.current_playlist = -1;
  s_screen.state.last_track_index = -2;

  build_view(s_screen);
}

void tick() {
  if (!screen_alive(&s_screen)) {
    return;
  }
  if (!s_screen.has_pending_intent) {
    return;
  }

  UiIntent intent = s_screen.pending_intent;
  s_screen.has_pending_intent = false;

  components::NavCommand cmd = components::handle_intent(s_screen, intent);
  switch (cmd.type) {
  case components::NavCommand::Type::NavigateTo:
    navigate_to(s_screen, cmd.target, true);
    break;
  case components::NavCommand::Type::Back:
    navigate_back(s_screen);
    break;
  case components::NavCommand::Type::Rebuild:
    rebuild_current(s_screen);
    break;
  default:
    break;
  }
}

void handle_media_key(MediaKey key) {
  if (!screen_alive(&s_screen)) {
    return;
  }
  if (!s_screen.player) {
    return;
  }

  UiIntent intent{};
  switch (key) {
  case MediaKey::PlayPause:
    intent.kind = UiIntentKind::TogglePause;
    break;
  case MediaKey::Next:
    intent.kind = UiIntentKind::NextTrack;
    break;
  case MediaKey::Prev:
    intent.kind = UiIntentKind::PrevTrack;
    break;
  default:
    return;
  }

  request_intent(&s_screen, intent);
}

void handle_global_key(GlobalKey key) {
  if (!screen_alive(&s_screen)) {
    return;
  }

  switch (key) {
  case GlobalKey::VolumeCycle: {
    if (!s_screen.player) {
      return;
    }
    uint8_t volume = app::player_get_volume(*s_screen.player);
    uint8_t next = (volume >= 21) ? 0 : static_cast<uint8_t>(volume + 1);
    app::player_set_volume(*s_screen.player, next);
    components::update_topbar(s_screen);
    break;
  }
  case GlobalKey::PlaybackModeCycle: {
    if (!s_screen.player) {
      return;
    }
    switch (s_screen.player->mode) {
    case app::PlaybackMode::Sequential:
      s_screen.player->mode = app::PlaybackMode::Shuffle;
      break;
    case app::PlaybackMode::Shuffle:
      s_screen.player->mode = app::PlaybackMode::RepeatOne;
      break;
    case app::PlaybackMode::RepeatOne:
    default:
      s_screen.player->mode = app::PlaybackMode::Sequential;
      break;
    }
    break;
  }
  case GlobalKey::DeletePrompt:
    show_delete_prompt(s_screen);
    break;
  case GlobalKey::DeleteConfirm:
    if (s_screen.delete_prompt_active) {
      perform_delete(s_screen);
    }
    break;
  case GlobalKey::DeleteCancel:
    if (s_screen.delete_prompt_active) {
      hide_delete_prompt(s_screen);
    }
    break;
  default:
    break;
  }
}

void rebuild() {
  if (!screen_alive(&s_screen)) {
    return;
  }
  rebuild_current(s_screen);
}

} // namespace lofi::ui
