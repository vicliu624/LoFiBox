#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::main_menu::layout {
struct MenuItemLayout {
  lv_obj_t *button = nullptr;
  lv_obj_t *icon = nullptr;
  lv_obj_t *label = nullptr;
};

struct MenuLayout {
  lv_obj_t *wrap = nullptr;
  lv_obj_t *arrow_left = nullptr;
  lv_obj_t *arrow_right = nullptr;
  lv_obj_t *dots[4] = {};
  MenuItemLayout items[3] = {};
  lv_coord_t icon_size = 0;
  lv_coord_t icon_top = 0;
  lv_coord_t icon_gap = 0;
  lv_coord_t label_top = 0;
  lv_coord_t label_height = 0;
  lv_coord_t row_left = 0;
  lv_coord_t item_height = 0;
};

constexpr int kVisibleItems = 3;
constexpr int kDots = 4;

MenuLayout create_menu(lv_obj_t *content);
void set_item(MenuItemLayout &item, const char *label,
              const lv_image_dsc_t *icon);
void set_dot_active(lv_obj_t *dot, bool active);

} // namespace lofi::ui::screens::main_menu::layout
