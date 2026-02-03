#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::eq::layout {
struct EqLayout {
  lv_obj_t *panel = nullptr;
  lv_obj_t *graph = nullptr;
  lv_obj_t *slider_cols[6] = {};
  lv_obj_t *sliders[6] = {};
  lv_obj_t *value_labels[6] = {};
  lv_obj_t *labels[12] = {};
  lv_obj_t *db_top = nullptr;
  lv_obj_t *db_mid = nullptr;
  lv_obj_t *db_bottom = nullptr;
  lv_obj_t *preset = nullptr;
  lv_obj_t *preset_value = nullptr;
  lv_obj_t *key_sink = nullptr;
};

EqLayout create(lv_obj_t *content);

} // namespace lofi::ui::screens::eq::layout
