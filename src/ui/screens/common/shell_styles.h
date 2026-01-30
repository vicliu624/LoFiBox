#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::common::styles
{
void init_once();

void apply_root(lv_obj_t* obj);
void apply_topbar(lv_obj_t* obj);
void apply_topbar_label(lv_obj_t* obj);
void apply_topbar_title(lv_obj_t* obj);
void apply_topbar_status(lv_obj_t* obj);

} // namespace lofi::ui::screens::common::styles
