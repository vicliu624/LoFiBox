#include "ui/screens/now_playing/now_playing_layout.h"

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
// +- key_sink

namespace lofi::ui::screens::now_playing::layout
{
NowPlayingLayout create_now_playing(lv_obj_t* content)
{
    NowPlayingLayout refs{};

    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(content, LV_LAYOUT_NONE);

    lv_coord_t content_w = lv_obj_get_width(content);
    lv_coord_t content_h = lv_obj_get_height(content);

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
    lv_obj_set_style_text_align(refs.time_right, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
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
    lv_obj_set_flex_align(refs.controls_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(refs.controls_row, LV_OBJ_FLAG_SCROLLABLE);

    refs.ctrl_prev = lv_label_create(refs.controls_row);
    refs.ctrl_play = lv_label_create(refs.controls_row);
    refs.ctrl_next = lv_label_create(refs.controls_row);
    refs.ctrl_shuffle = lv_label_create(refs.controls_row);
    refs.ctrl_repeat = lv_label_create(refs.controls_row);

    refs.key_sink = lv_btn_create(content);
    lv_obj_set_size(refs.key_sink, 1, 1);
    lv_obj_clear_flag(refs.key_sink, LV_OBJ_FLAG_SCROLLABLE);

    return refs;
}

} // namespace lofi::ui::screens::now_playing::layout
