#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::list_page::styles
{
void init_once();

void apply_content(lv_obj_t* obj);
void apply_list(lv_obj_t* obj);
void apply_list_row(lv_obj_t* obj);
void apply_list_label_left(lv_obj_t* obj);
void apply_list_label_right(lv_obj_t* obj);

void apply_checked_state(lv_obj_t* obj);
void clear_checked_state(lv_obj_t* obj);

} // namespace lofi::ui::screens::list_page::styles
