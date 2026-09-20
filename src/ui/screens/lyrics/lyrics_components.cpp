#include "ui/screens/lyrics/lyrics_components.h"

#include "app/lyrics.h"
#include "app/player.h"
#include "app/network.h"
#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::lyrics {
namespace {
constexpr int kVisibleLines = 5;
constexpr int kActiveSlot = kVisibleLines / 2;

struct LyricsView {
  lv_obj_t *title = nullptr;
  lv_obj_t *viewport = nullptr;
  lv_obj_t *line_group = nullptr;
  lv_obj_t *lines[kVisibleLines] = {};
  lv_obj_t *empty = nullptr;
  lv_obj_t *key_sink = nullptr;
  lv_coord_t row_h = 0;
  int active_line = -2;
  uint32_t lyrics_version = 0;
};

LyricsView s_view;

void apply_label(lv_obj_t *label, lv_color_t color) {
  lv_obj_set_style_text_font(label, font_noto_sc_16(), LV_PART_MAIN);
  lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(label, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(label, 0, LV_PART_MAIN);
}

void fill_lines(int active) {
  for (int slot = 0; slot < kVisibleLines; ++slot) {
    const int index = active + slot - kActiveSlot;
    lv_obj_t *label = s_view.lines[slot];
    if (!label) {
      continue;
    }
    lv_label_set_text(label, app::lyrics_line_text(index));
    const int distance = index - active;
    const lv_color_t color = distance == 0
                                 ? lv_color_hex(0xffffff)
                                 : (distance >= -1 && distance <= 1)
                                       ? lv_color_hex(0xc6ccd4)
                                       : lv_color_hex(0x717780);
    lv_obj_set_style_text_color(label, color, LV_PART_MAIN);
  }
}

void animate_to_rest(lv_coord_t start_y) {
  lv_anim_del(s_view.line_group, nullptr);
  lv_obj_set_y(s_view.line_group, start_y);
  lv_anim_t animation;
  lv_anim_init(&animation);
  lv_anim_set_var(&animation, s_view.line_group);
  lv_anim_set_values(&animation, start_y, 0);
  lv_anim_set_time(&animation, 180);
  lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
  lv_anim_set_exec_cb(&animation, [](void *target, int32_t value) {
    lv_obj_set_y(static_cast<lv_obj_t *>(target), static_cast<lv_coord_t>(value));
  });
  lv_anim_start(&animation);
}

void key_cb(lv_event_t *event) {
  auto *screen = static_cast<UiScreen *>(lv_event_get_user_data(event));
  if (!screen_alive(screen) || lv_event_get_code(event) != LV_EVENT_KEY) {
    return;
  }
  UiIntent intent{};
  const uint32_t key = lv_event_get_key(event);
  if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
    intent.kind = UiIntentKind::NavigateBack;
  } else if (key == LV_KEY_DOWN) {
    // Lyrics is a Now Playing mode: the same shortcut toggles back to player.
    intent.kind = UiIntentKind::NavigateBack;
  } else if (key == LV_KEY_LEFT) {
    intent.kind = UiIntentKind::PrevTrack;
  } else if (key == LV_KEY_RIGHT) {
    intent.kind = UiIntentKind::NextTrack;
  } else if (key == LV_KEY_ENTER) {
    intent.kind = UiIntentKind::TogglePause;
  } else {
    return;
  }
  request_intent(screen, intent);
}
} // namespace

void build(UiScreen &screen) {
  s_view = {};
  lv_obj_t *content = screen.view.root.content;
  if (!content) {
    return;
  }
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(content, lv_color_hex(0x0d0f12), LV_PART_MAIN);
  lv_obj_set_style_bg_grad_color(content, lv_color_hex(0x08090b), LV_PART_MAIN);
  lv_obj_set_style_bg_grad_dir(content, LV_GRAD_DIR_VER, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(content, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(content, 0, LV_PART_MAIN);
  lv_obj_set_style_border_width(content, 0, LV_PART_MAIN);

  const lv_coord_t width = lv_obj_get_width(content);
  const lv_coord_t height = lv_obj_get_height(content);
  const lv_coord_t side_pad = width <= 360 ? 10 : 18;
  const lv_coord_t title_h = 24;
  const lv_coord_t viewport_y = title_h + 6;
  const lv_coord_t viewport_h = height - viewport_y - 12;
  s_view.row_h = viewport_h / kVisibleLines;
  if (s_view.row_h < 22) {
    s_view.row_h = 22;
  }

  s_view.title = lv_label_create(content);
  lv_obj_set_pos(s_view.title, side_pad, 2);
  lv_obj_set_width(s_view.title, width - side_pad * 2);
  lv_label_set_long_mode(s_view.title, LV_LABEL_LONG_DOT);
  apply_label(s_view.title, lv_color_hex(0x9da5af));

  s_view.viewport = lv_obj_create(content);
  lv_obj_set_pos(s_view.viewport, side_pad, viewport_y);
  lv_obj_set_size(s_view.viewport, width - side_pad * 2, viewport_h);
  lv_obj_clear_flag(s_view.viewport, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(s_view.viewport, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(s_view.viewport, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(s_view.viewport, 0, LV_PART_MAIN);

  s_view.line_group = lv_obj_create(s_view.viewport);
  lv_obj_set_size(s_view.line_group, LV_PCT(100), viewport_h);
  lv_obj_set_pos(s_view.line_group, 0, 0);
  lv_obj_clear_flag(s_view.line_group, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(s_view.line_group, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(s_view.line_group, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(s_view.line_group, 0, LV_PART_MAIN);

  for (int slot = 0; slot < kVisibleLines; ++slot) {
    s_view.lines[slot] = lv_label_create(s_view.line_group);
    lv_obj_set_pos(s_view.lines[slot], 0, slot * s_view.row_h);
    lv_obj_set_size(s_view.lines[slot], LV_PCT(100), s_view.row_h);
    lv_label_set_long_mode(s_view.lines[slot], LV_LABEL_LONG_DOT);
    apply_label(s_view.lines[slot], lv_color_hex(0x717780));
  }

  s_view.empty = lv_label_create(content);
  lv_obj_set_width(s_view.empty, width - side_pad * 2);
  lv_obj_align(s_view.empty, LV_ALIGN_CENTER, 0, 8);
  apply_label(s_view.empty, lv_color_hex(0x9da5af));

  s_view.key_sink = lv_btn_create(content);
  lv_obj_set_size(s_view.key_sink, 1, 1);
  lv_obj_add_flag(s_view.key_sink, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(s_view.key_sink, key_cb, LV_EVENT_KEY, &screen);
  if (screen.group) {
    lv_group_add_obj(screen.group, s_view.key_sink);
    lv_group_focus_obj(s_view.key_sink);
  }
  update(screen);
}

void update(UiScreen &screen) {
  if (screen.state.current != PageId::Lyrics || !s_view.title) {
    return;
  }
  const int track_index = screen.player ? screen.player->current_index : -1;
  const bool has_track = screen.library && track_index >= 0 &&
                         track_index < screen.library->track_count;
  const app::LyricsState &lyrics = app::lyrics_state();
  if (has_track) {
    const char *title = screen.library->tracks[track_index].title;
    lv_label_set_text(s_view.title, title && title[0] ? title : "Lyrics");
  } else {
    lv_label_set_text(s_view.title, "Lyrics");
  }
  if (!has_track || !lyrics.synchronized || lyrics.track_index != track_index) {
    lv_obj_add_flag(s_view.viewport, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(s_view.empty, LV_OBJ_FLAG_HIDDEN);
    lv_label_set_text(s_view.empty,
                      has_track
                          ? (app::network::connected()
                                 ? "Lyrics unavailable\nReturn to Now Playing and press Up to download"
                                 : "Wi-Fi is not connected\nOpen Settings > Wi-Fi setup")
                                : "No track selected");
    return;
  }
  lv_obj_clear_flag(s_view.viewport, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(s_view.empty, LV_OBJ_FLAG_HIDDEN);
  const int active = app::lyrics_active_line(app::player_current_time_ms());
  if (active == s_view.active_line && lyrics.version == s_view.lyrics_version) {
    return;
  }
  const int previous = s_view.active_line;
  s_view.active_line = active;
  s_view.lyrics_version = lyrics.version;
  fill_lines(active);
  if (previous >= 0 && active != previous) {
    animate_to_rest(active > previous ? s_view.row_h : -s_view.row_h);
  } else {
    lv_anim_del(s_view.line_group, nullptr);
    lv_obj_set_y(s_view.line_group, 0);
  }
}

} // namespace lofi::ui::screens::lyrics
