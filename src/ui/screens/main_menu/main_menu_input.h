#pragma once

#include "ui/lofibox/lofibox_ui_internal.h"

namespace lofi::ui::screens::main_menu::input {
void attach_row(UiScreen &screen, RowMeta &meta);
void focus_first(lv_group_t *group, lv_obj_t *first);

} // namespace lofi::ui::screens::main_menu::input
