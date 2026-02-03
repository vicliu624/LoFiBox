#include "ui/screens/common/shell_layout.h"

// Wireframe (Shell)
// +----------------------------------------------+
// | < Back              TITLE     WIFI     BAT   |  topbar
// +----------------------------------------------+
// |                                              |
// |                 CONTENT                      |
// |                                              |
// +----------------------------------------------+
//
// Tree (Shell)
// root
// +- topbar
// |  +- top_left
// |  +- top_title
// |  +- top_status
// |     +- top_signal
// |     +- top_battery
// +- content

namespace lofi::ui::screens::common::layout {
namespace {
constexpr lv_coord_t kBaseScreenHeight = 222;
constexpr lv_coord_t kBaseTopbarHeight = 26;
constexpr lv_coord_t kMinTopbarHeight = 16;

lv_coord_t scale_from_base(lv_coord_t value, lv_coord_t screen_h) {
  if (screen_h <= 0) {
    return value;
  }
  return static_cast<lv_coord_t>((value * screen_h) / kBaseScreenHeight);
}
} // namespace

lv_coord_t topbar_height() {
  lv_coord_t h = lv_display_get_vertical_resolution(nullptr);
  lv_coord_t scaled = scale_from_base(kBaseTopbarHeight, h);
  return (scaled < kMinTopbarHeight) ? kMinTopbarHeight : scaled;
}

RootLayout create_root() {
  RootLayout refs{};

  refs.root = lv_obj_create(lv_screen_active());
  lv_obj_set_size(refs.root, LV_PCT(100), LV_PCT(100));
  lv_obj_clear_flag(refs.root, LV_OBJ_FLAG_SCROLLABLE);

  refs.topbar = lv_obj_create(refs.root);
  lv_obj_set_size(refs.topbar, LV_PCT(100), topbar_height());
  lv_obj_align(refs.topbar, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_clear_flag(refs.topbar, LV_OBJ_FLAG_SCROLLABLE);

  refs.top_left = lv_label_create(refs.topbar);
  lv_obj_align(refs.top_left, LV_ALIGN_LEFT_MID, 0, 0);

  refs.top_title_wrap = lv_obj_create(refs.topbar);
  lv_obj_set_size(refs.top_title_wrap, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_align(refs.top_title_wrap, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(refs.top_title_wrap, LV_OBJ_FLAG_SCROLLABLE);

  refs.top_title = lv_label_create(refs.top_title_wrap);
  lv_obj_align(refs.top_title, LV_ALIGN_LEFT_MID, 0, 0);

  refs.top_title_fade_left = lv_obj_create(refs.top_title_wrap);
  lv_obj_set_size(refs.top_title_fade_left, 10, topbar_height());
  lv_obj_align(refs.top_title_fade_left, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_clear_flag(refs.top_title_fade_left, LV_OBJ_FLAG_SCROLLABLE);

  refs.top_title_fade_right = lv_obj_create(refs.top_title_wrap);
  lv_obj_set_size(refs.top_title_fade_right, 10, topbar_height());
  lv_obj_align(refs.top_title_fade_right, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_clear_flag(refs.top_title_fade_right, LV_OBJ_FLAG_SCROLLABLE);

  refs.top_status = lv_obj_create(refs.topbar);
  lv_obj_set_size(refs.top_status, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(refs.top_status, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(refs.top_status, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_clear_flag(refs.top_status, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(refs.top_status, LV_ALIGN_RIGHT_MID, 0, 0);

  refs.top_signal = lv_label_create(refs.top_status);
  refs.top_battery = lv_label_create(refs.top_status);

  refs.content = lv_obj_create(refs.root);
  lv_coord_t content_h =
      lv_display_get_vertical_resolution(nullptr) - topbar_height();
  lv_obj_set_size(refs.content, LV_PCT(100), content_h);
  lv_obj_align(refs.content, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_clear_flag(refs.content, LV_OBJ_FLAG_SCROLLABLE);

  return refs;
}

} // namespace lofi::ui::screens::common::layout
