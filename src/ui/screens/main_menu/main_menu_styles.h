#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::main_menu::styles
{
void init_once();

void apply_content(lv_obj_t* obj);
void apply_item(lv_obj_t* obj);
void apply_label(lv_obj_t* obj);
void apply_arrow(lv_obj_t* obj);
void apply_dot(lv_obj_t* obj);
void apply_checked_state(lv_obj_t* obj);
void clear_checked_state(lv_obj_t* obj);

} // namespace lofi::ui::screens::main_menu::styles
