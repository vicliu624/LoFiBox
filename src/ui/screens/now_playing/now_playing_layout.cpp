#include "ui/screens/now_playing/now_playing_layout.h"

#include <Arduino.h>

// Wireframe (Now Playing)
// +----------------------------------------------+
// | < Back           Now Playing        WIFI BAT |
// +----------------------------------------------+
// | [Cover]  Track Title                         |
// |          Artist Name                         |
// |          Album Name                          |
// |          01:23  --------●---------      03:58 |
// |                                              |
// |          [<<] [>||] [>>] [~] [∞]             |
// +----------------------------------------------+
//
// Tree (Now Playing)
// content
// +- cover
// +- title
// +- artist
// +- album
// +- time_left
// +- time_right
// +- bar_wrap
// |  +- bar
// |  +- knob
// +- controls_row
// |  +- ctrl_prev
// |  +- ctrl_play
// |  +- ctrl_next
// |  +- ctrl_shuffle
// |  +- ctrl_repeat
// +- lyrics_hint (DOWN toggles the temporary lyrics mode)
// +- key_sink

namespace lofi::ui::screens::now_playing::layout {
NowPlayingLayout create_now_playing(lv_obj_t *content) {
  NowPlayingLayout refs{};

  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_layout(content, LV_LAYOUT_NONE);

  lv_coord_t content_w = lv_obj_get_width(content);
  lv_coord_t content_h = lv_obj_get_height(content);

#if defined(BOARD_M5STACK_CORE2_AUDIO_FACES)
  // Core2 is a 320x240 landscape touch display (212px after the shell's
  // topbar), not a narrow version of Pager's 480x222 composition.  A compact,
  // centered cover lets the track metadata, seek position and large touch
  // transport targets each retain their own vertical band.
  // Do not depend on LVGL's percentage-sized content object having completed
  // its first layout pass here.  The Core2 build always uses this composition;
  // the fallback dimensions only affect the pre-layout construction moment.
  if (content_w <= 0) {
    content_w = 320;
  }
  if (content_h <= 0) {
    content_h = 212;
  }
  {
    constexpr lv_coord_t kSideInset = 12;
    constexpr lv_coord_t kCoverSize = 82;
    constexpr lv_coord_t kCoverTop = 5;
    constexpr lv_coord_t kTitleTop = 91;
    constexpr lv_coord_t kArtistTop = 109;
    constexpr lv_coord_t kAlbumTop = 127;
    constexpr lv_coord_t kBarTop = 148;
    constexpr lv_coord_t kTimeTop = 159;
    constexpr lv_coord_t kControlsTop = 176;
    constexpr lv_coord_t kControlsHeight = 24;
    constexpr lv_coord_t kLyricsTop = 198;

    const lv_coord_t usable_w = content_w - (kSideInset * 2);
    refs.cover_size = kCoverSize;
    refs.bar_width = usable_w;

    refs.cover = lv_canvas_create(content);
    lv_obj_set_pos(refs.cover, (content_w - kCoverSize) / 2, kCoverTop);
    lv_obj_set_size(refs.cover, kCoverSize, kCoverSize);

    refs.title = lv_label_create(content);
    lv_label_set_long_mode(refs.title, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(refs.title, kSideInset, kTitleTop);
    lv_obj_set_width(refs.title, usable_w);

    refs.artist = lv_label_create(content);
    lv_label_set_long_mode(refs.artist, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(refs.artist, kSideInset, kArtistTop);
    lv_obj_set_width(refs.artist, usable_w);

    refs.album = lv_label_create(content);
    lv_label_set_long_mode(refs.album, LV_LABEL_LONG_DOT);
    lv_obj_set_pos(refs.album, kSideInset, kAlbumTop);
    lv_obj_set_width(refs.album, usable_w);

    refs.bar_wrap = lv_obj_create(content);
    lv_obj_set_size(refs.bar_wrap, usable_w, 10);
    lv_obj_set_pos(refs.bar_wrap, kSideInset, kBarTop);
    lv_obj_clear_flag(refs.bar_wrap, LV_OBJ_FLAG_SCROLLABLE);
    refs.bar = lv_bar_create(refs.bar_wrap);
    lv_obj_set_size(refs.bar, usable_w, 5);
    lv_obj_align(refs.bar, LV_ALIGN_CENTER, 0, 0);
    refs.knob = lv_obj_create(refs.bar_wrap);
    lv_obj_set_size(refs.knob, 3, 12);
    lv_obj_align(refs.knob, LV_ALIGN_LEFT_MID, 0, 0);

    refs.time_left = lv_label_create(content);
    lv_obj_set_pos(refs.time_left, kSideInset, kTimeTop);
    refs.time_right = lv_label_create(content);
    lv_obj_set_pos(refs.time_right, content_w - kSideInset - 42, kTimeTop);
    lv_obj_set_width(refs.time_right, 42);
    lv_obj_set_style_text_align(refs.time_right, LV_TEXT_ALIGN_RIGHT,
                                LV_PART_MAIN);
    lv_label_set_long_mode(refs.time_right, LV_LABEL_LONG_CLIP);

    refs.controls_row = lv_obj_create(content);
    lv_obj_set_size(refs.controls_row, usable_w, kControlsHeight);
    lv_obj_set_pos(refs.controls_row, kSideInset, kControlsTop);
    lv_obj_set_flex_flow(refs.controls_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(refs.controls_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(refs.controls_row, LV_OBJ_FLAG_SCROLLABLE);
    refs.ctrl_prev = lv_label_create(refs.controls_row);
    refs.ctrl_play = lv_label_create(refs.controls_row);
    refs.ctrl_next = lv_label_create(refs.controls_row);
    refs.ctrl_shuffle = lv_label_create(refs.controls_row);
    refs.ctrl_repeat = lv_label_create(refs.controls_row);
    lv_obj_t *controls[] = {refs.ctrl_prev, refs.ctrl_play, refs.ctrl_next,
                            refs.ctrl_shuffle, refs.ctrl_repeat};
    for (auto *control : controls) {
      // The glyph itself is visually small, but each action receives a 40px
      // wide hit target, appropriate for a finger rather than a D-pad.
      lv_obj_set_size(control, 40, kControlsHeight);
      lv_obj_set_style_text_align(control, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    refs.lyrics_hint = lv_label_create(content);
    lv_obj_set_pos(refs.lyrics_hint, kSideInset, kLyricsTop);
    lv_obj_set_width(refs.lyrics_hint, usable_w);
    lv_obj_set_style_text_align(refs.lyrics_hint, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN);
    lv_label_set_long_mode(refs.lyrics_hint, LV_LABEL_LONG_CLIP);

    refs.key_sink = lv_btn_create(content);
    lv_obj_set_size(refs.key_sink, 1, 1);
    lv_obj_clear_flag(refs.key_sink, LV_OBJ_FLAG_SCROLLABLE);
    Serial.printf("[CORE2] Now Playing compact layout %dx%d\n", content_w,
                  content_h);
    return refs;
  }
#endif

  constexpr lv_coord_t kBaseW = 480;
  constexpr lv_coord_t kBaseH = 196;
  auto scale = [content_w](lv_coord_t value) -> lv_coord_t {
    if (content_w <= 0) {
      return value;
    }
    return static_cast<lv_coord_t>((value * content_w) / kBaseW);
  };

  lv_coord_t design_h = scale(kBaseH);
  lv_coord_t offset_y = 0;
  if (content_h > design_h) {
    offset_y = (content_h - design_h) / 2;
  }

  lv_coord_t cover_delta = scale(20);
  lv_coord_t cover_x = scale(36) - cover_delta;
  lv_coord_t cover_y = offset_y + scale(42) - cover_delta;
  lv_coord_t cover_size = scale(151);
  if (cover_size < 24) {
    cover_size = 24;
  }
  refs.cover_size = cover_size;

  lv_coord_t panel_x = scale(26);
  lv_coord_t panel_y = offset_y + scale(34);
  lv_coord_t panel_w = scale(430);
  lv_coord_t panel_h = scale(158);
  (void)panel_y;
  (void)panel_h;
  lv_coord_t meta_x = cover_x + cover_size + scale(14);
  lv_coord_t pad_right = scale(16);
  lv_coord_t meta_w = (panel_x + panel_w) - meta_x - pad_right;
  lv_coord_t title_y = offset_y + scale(53) - 14;
  lv_coord_t artist_y = offset_y + scale(78) - 14;
  lv_coord_t album_y = offset_y + scale(96) - 14;
  lv_coord_t bar_shift = 8;
  lv_coord_t bar_y = offset_y + scale(110) + bar_shift;
  lv_coord_t time_bottom_y = offset_y + scale(124) + bar_shift;
  lv_coord_t controls_y = offset_y + scale(155) + bar_shift;

  lv_coord_t bar_x = meta_x;
  lv_coord_t time_width = scale(36);
  if (time_width < 24) {
    time_width = 24;
  }
  lv_coord_t time_right_extra = 4;
  time_width += time_right_extra;
  lv_coord_t time_right_x = panel_x + panel_w - pad_right - time_width;
  lv_coord_t bar_width = (time_right_x + time_width) - bar_x;
  if (bar_width < scale(80)) {
    bar_width = scale(80);
  }
  refs.bar_width = bar_width;

  lv_coord_t bar_height = scale(6);
  if (bar_height < 4) {
    bar_height = 4;
  }
  lv_coord_t knob_h = bar_height + scale(7);
  lv_coord_t bar_wrap_height = knob_h;
  lv_coord_t controls_h = scale(24);
  if (controls_h < 18) {
    controls_h = 18;
  }

  refs.cover = lv_canvas_create(content);
  lv_obj_set_pos(refs.cover, cover_x, cover_y);
  lv_obj_set_size(refs.cover, cover_size, cover_size);

  refs.title = lv_label_create(content);
  lv_label_set_long_mode(refs.title, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(refs.title, meta_x, title_y);
  lv_obj_set_width(refs.title, meta_w);

  refs.artist = lv_label_create(content);
  lv_label_set_long_mode(refs.artist, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(refs.artist, meta_x, artist_y);
  lv_obj_set_width(refs.artist, meta_w);

  refs.album = lv_label_create(content);
  lv_label_set_long_mode(refs.album, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(refs.album, meta_x, album_y);
  lv_obj_set_width(refs.album, meta_w);

  refs.time_left = lv_label_create(content);
  lv_obj_set_pos(refs.time_left, meta_x, time_bottom_y);

  refs.time_right = lv_label_create(content);
  lv_obj_set_pos(refs.time_right, time_right_x, time_bottom_y);
  lv_obj_set_width(refs.time_right, time_width);
  lv_obj_set_style_text_align(refs.time_right, LV_TEXT_ALIGN_RIGHT,
                              LV_PART_MAIN);
  lv_label_set_long_mode(refs.time_right, LV_LABEL_LONG_CLIP);

  refs.bar_wrap = lv_obj_create(content);
  lv_obj_set_size(refs.bar_wrap, bar_width, bar_wrap_height);
  lv_obj_set_pos(refs.bar_wrap, bar_x, bar_y);
  lv_obj_clear_flag(refs.bar_wrap, LV_OBJ_FLAG_SCROLLABLE);

  refs.bar = lv_bar_create(refs.bar_wrap);
  lv_obj_set_size(refs.bar, bar_width, bar_height);
  lv_obj_align(refs.bar, LV_ALIGN_CENTER, 0, 0);

  lv_coord_t knob_w = scale(3);
  if (knob_w < 2) {
    knob_w = 2;
  }
  refs.knob = lv_obj_create(refs.bar_wrap);
  lv_obj_set_size(refs.knob, knob_w, knob_h);
  lv_obj_align(refs.knob, LV_ALIGN_LEFT_MID, 0, 0);

  refs.controls_row = lv_obj_create(content);
  lv_obj_set_size(refs.controls_row, meta_w, controls_h);
  lv_obj_set_pos(refs.controls_row, meta_x, controls_y);
  lv_obj_set_flex_flow(refs.controls_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(refs.controls_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(refs.controls_row, LV_OBJ_FLAG_SCROLLABLE);

  refs.ctrl_prev = lv_label_create(refs.controls_row);
  refs.ctrl_play = lv_label_create(refs.controls_row);
  refs.ctrl_next = lv_label_create(refs.controls_row);
  refs.ctrl_shuffle = lv_label_create(refs.controls_row);
  refs.ctrl_repeat = lv_label_create(refs.controls_row);

  refs.lyrics_hint = lv_label_create(content);
  lv_obj_set_pos(refs.lyrics_hint, meta_x, controls_y + controls_h + scale(8));
  lv_obj_set_width(refs.lyrics_hint, meta_w);
  lv_obj_set_style_text_align(refs.lyrics_hint, LV_TEXT_ALIGN_CENTER,
                              LV_PART_MAIN);
  lv_label_set_long_mode(refs.lyrics_hint, LV_LABEL_LONG_CLIP);

  refs.key_sink = lv_btn_create(content);
  lv_obj_set_size(refs.key_sink, 1, 1);
  lv_obj_clear_flag(refs.key_sink, LV_OBJ_FLAG_SCROLLABLE);

  return refs;
}

} // namespace lofi::ui::screens::now_playing::layout
