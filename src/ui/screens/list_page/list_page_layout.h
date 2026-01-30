#pragma once

#include <lvgl.h>

namespace lofi::ui::screens::list_page::layout
{
struct ListLayout
{
    lv_obj_t* list = nullptr;
};

struct ListRowLayout
{
    lv_obj_t* row = nullptr;
    lv_obj_t* icon = nullptr;
    lv_obj_t* left_label = nullptr;
    lv_obj_t* right_label = nullptr;
};

lv_coord_t row_height();
ListLayout create_list(lv_obj_t* content);
ListRowLayout create_list_row(lv_obj_t* list);
void set_row_icon_visible(ListRowLayout& row, bool visible);

} // namespace lofi::ui::screens::list_page::layout
