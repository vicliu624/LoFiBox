#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::eq::styles {
void init_once();
void apply_content(lv_obj_t *obj);
void apply_panel(lv_obj_t *obj);
void apply_graph(lv_obj_t *obj);
void apply_db_label(lv_obj_t *obj);
void apply_slider(lv_obj_t *obj);
void apply_slider_selected(lv_obj_t *obj);
void apply_label(lv_obj_t *obj);
void apply_value_label(lv_obj_t *obj);
void apply_preset(lv_obj_t *obj);
void apply_preset_value(lv_obj_t *obj);

} // namespace lofi::ui::screens::eq::styles
