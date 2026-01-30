#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::now_playing::layout
{
struct NowPlayingLayout
{
    lv_obj_t* cover = nullptr;
    lv_obj_t* title = nullptr;
    lv_obj_t* artist = nullptr;
    lv_obj_t* album = nullptr;
    lv_obj_t* time_left = nullptr;
    lv_obj_t* time_right = nullptr;
    lv_obj_t* bar_wrap = nullptr;
    lv_obj_t* bar = nullptr;
    lv_obj_t* knob = nullptr;
    lv_obj_t* controls_row = nullptr;
    lv_obj_t* ctrl_prev = nullptr;
    lv_obj_t* ctrl_play = nullptr;
    lv_obj_t* ctrl_next = nullptr;
    lv_obj_t* ctrl_shuffle = nullptr;
    lv_obj_t* ctrl_repeat = nullptr;
    lv_obj_t* key_sink = nullptr;
    lv_coord_t bar_width = 0;
    lv_coord_t cover_size = 0;
};

NowPlayingLayout create_now_playing(lv_obj_t* content);

} // namespace lofi::ui::screens::now_playing::layout
