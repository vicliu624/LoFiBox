#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::common::layout
{
struct RootLayout
{
    lv_obj_t* root = nullptr;
    lv_obj_t* topbar = nullptr;
    lv_obj_t* content = nullptr;
    lv_obj_t* top_left = nullptr;
    lv_obj_t* top_title = nullptr;
    lv_obj_t* top_title_fade_left = nullptr;
    lv_obj_t* top_title_fade_right = nullptr;
    lv_obj_t* top_status = nullptr;
    lv_obj_t* top_signal = nullptr;
    lv_obj_t* top_battery = nullptr;
};

lv_coord_t topbar_height();
RootLayout create_root();

} // namespace lofi::ui::screens::common::layout
