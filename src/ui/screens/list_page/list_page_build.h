#pragma once

#include "ui/lofibox/lofibox_ui_internal.h"
#include "ui/screens/list_page/list_page_layout.h"

namespace lofi::ui::screens::list_page {
struct BuildApi {
  void (*init_styles)() = nullptr;
  void (*apply_content)(lv_obj_t *) = nullptr;
  void (*apply_list)(lv_obj_t *) = nullptr;
  void (*apply_row)(lv_obj_t *) = nullptr;
  void (*apply_left)(lv_obj_t *) = nullptr;
  void (*apply_right)(lv_obj_t *) = nullptr;
  list_page::layout::ListLayout (*create_list)(lv_obj_t *) = nullptr;
  list_page::layout::ListRowLayout (*create_row)(lv_obj_t *) = nullptr;
  void (*attach_row)(UiScreen &, RowMeta &) = nullptr;
  void (*focus_first)(lv_group_t *, lv_obj_t *) = nullptr;
};

BuildApi api_for(PageId id);
void populate_list(UiScreen &screen);
void build_list_page(UiScreen &screen, const BuildApi &api);
} // namespace lofi::ui::screens::list_page
