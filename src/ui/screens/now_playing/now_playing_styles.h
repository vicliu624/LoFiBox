#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::now_playing::styles
{
void init_once();

void apply_content(lv_obj_t* obj);
void apply_cover(lv_obj_t* obj);
void apply_title(lv_obj_t* obj);
void apply_subtitle(lv_obj_t* obj);
void apply_time_label(lv_obj_t* obj);
void apply_bar_wrap(lv_obj_t* obj);
void apply_bar(lv_obj_t* obj);
void apply_knob(lv_obj_t* obj);
void apply_controls_row(lv_obj_t* obj);
void apply_control_icon(lv_obj_t* obj);
void apply_key_sink(lv_obj_t* obj);

} // namespace lofi::ui::screens::now_playing::styles
