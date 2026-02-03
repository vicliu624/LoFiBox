#pragma once

#include "ui/lofibox/lofibox_ui_internal.h"

namespace lofi::ui::screens::list_page::input {
void attach_row(UiScreen &screen, RowMeta &meta);
void focus_first(lv_group_t *group, lv_obj_t *first);
void refresh_rows(UiScreen &screen);
void move_selection(UiScreen &screen, int delta);

} // namespace lofi::ui::screens::list_page::input
