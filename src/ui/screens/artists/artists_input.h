#pragma once

#include "ui/screens/list_page/list_page_input.h"

namespace lofi::ui::screens::artists::input {
void attach_row(UiScreen &screen, RowMeta &meta);
void focus_first(lv_group_t *group, lv_obj_t *first);

} // namespace lofi::ui::screens::artists::input
