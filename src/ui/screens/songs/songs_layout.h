#pragma once

#include "ui/screens/list_page/list_page_layout.h"

namespace lofi::ui::screens::songs::layout {
using ListLayout = ::lofi::ui::screens::list_page::layout::ListLayout;
using ListRowLayout = ::lofi::ui::screens::list_page::layout::ListRowLayout;

lv_coord_t row_height();
ListLayout create_list(lv_obj_t *content);
ListRowLayout create_list_row(lv_obj_t *list);

} // namespace lofi::ui::screens::songs::layout
