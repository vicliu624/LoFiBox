#include "ui/screens/list_page/list_page_layout.h"

// Wireframe (List Page)
// +----------------------------------------------+
// | < Back              TITLE               BAT  |  topbar
// +----------------------------------------------+
// |  [icon] Row 1                                 |
// |  [icon] Row 2                                 |
// |  Row 3                                        |
// |  Row 4                                        |
// |  ...                                          |
// +----------------------------------------------+
//
// Tree (List Page)
// content
// +- list
//    +- row
//    |  +- icon
//    |  +- left_label
//    |  +- right_label
//    +- row
//    +- ...

namespace lofi::ui::screens::list_page::layout {
namespace {
constexpr lv_coord_t kBaseScreenHeight = 222;
constexpr lv_coord_t kBaseScreenWidth = 480;
constexpr lv_coord_t kBaseRowHeight = 24;
constexpr lv_coord_t kBaseIconSize = 18;
constexpr lv_coord_t kBasePadLeft = 14;
constexpr lv_coord_t kBasePadRight = 8;
constexpr lv_coord_t kBaseIconGap = 8;
constexpr lv_coord_t kMinRowHeight = 16;
constexpr lv_coord_t kMinIconSize = 12;

lv_coord_t scale_h(lv_coord_t value) {
  lv_coord_t h = lv_display_get_vertical_resolution(nullptr);
  if (h <= 0) {
    return value;
  }
  return static_cast<lv_coord_t>((value * h) / kBaseScreenHeight);
}

lv_coord_t scale_w(lv_coord_t value) {
  lv_coord_t w = lv_display_get_horizontal_resolution(nullptr);
  if (w <= 0) {
    return value;
  }
  return static_cast<lv_coord_t>((value * w) / kBaseScreenWidth);
}

lv_coord_t icon_size() {
  lv_coord_t size = scale_w(kBaseIconSize);
  return (size < kMinIconSize) ? kMinIconSize : size;
}

lv_coord_t pad_left() { return scale_w(kBasePadLeft); }

lv_coord_t pad_right() { return scale_w(kBasePadRight); }

lv_coord_t icon_gap() { return scale_w(kBaseIconGap); }
} // namespace

lv_coord_t row_height() {
  lv_coord_t height = scale_h(kBaseRowHeight);
  return (height < kMinRowHeight) ? kMinRowHeight : height;
}

ListLayout create_list(lv_obj_t *content) {
  ListLayout refs{};
  refs.list = lv_obj_create(content);
  lv_obj_set_size(refs.list, LV_PCT(100), LV_PCT(100));
  lv_obj_align(refs.list, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(refs.list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_scroll_dir(refs.list, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(refs.list, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(refs.list, LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_clear_flag(refs.list, LV_OBJ_FLAG_SCROLL_MOMENTUM);
  return refs;
}

ListRowLayout create_list_row(lv_obj_t *list) {
  ListRowLayout refs{};
  refs.row = lv_btn_create(list);
  lv_obj_set_size(refs.row, LV_PCT(100), row_height());
  lv_obj_clear_flag(refs.row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(refs.row, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  lv_coord_t icon = icon_size();
  lv_coord_t left = pad_left();
  lv_coord_t right = pad_right();
  lv_coord_t gap = icon_gap();

  refs.icon = lv_img_create(refs.row);
  lv_obj_set_size(refs.icon, icon, icon);
  lv_obj_align(refs.icon, LV_ALIGN_LEFT_MID, left, 0);

  refs.left_label = lv_label_create(refs.row);
  lv_label_set_long_mode(refs.left_label, LV_LABEL_LONG_DOT);
  lv_obj_set_width(refs.left_label, LV_PCT(70));
  lv_obj_align(refs.left_label, LV_ALIGN_LEFT_MID, left + icon + gap, 0);

  refs.right_label = lv_label_create(refs.row);
  lv_label_set_long_mode(refs.right_label, LV_LABEL_LONG_CLIP);
  lv_obj_set_width(refs.right_label, LV_PCT(20));
  lv_obj_align(refs.right_label, LV_ALIGN_RIGHT_MID, -right, 0);

  return refs;
}

void set_row_icon_visible(ListRowLayout &row, bool visible) {
  if (!row.row || !row.left_label || !row.icon) {
    return;
  }
  lv_coord_t icon = icon_size();
  lv_coord_t left = pad_left();
  lv_coord_t gap = icon_gap();
  if (visible) {
    lv_obj_set_size(row.icon, icon, icon);
    lv_obj_align(row.icon, LV_ALIGN_LEFT_MID, left, 0);
    lv_obj_align(row.left_label, LV_ALIGN_LEFT_MID, left + icon + gap, 0);
  } else {
    lv_obj_set_size(row.icon, 0, 0);
    lv_obj_align(row.left_label, LV_ALIGN_LEFT_MID, left, 0);
  }
}

} // namespace lofi::ui::screens::list_page::layout
