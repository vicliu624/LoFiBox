#include "ui/screens/about/about_styles.h"

#include "ui/screens/list_page/list_page_styles.h"

namespace lofi::ui::screens::about::styles {
void init_once() { list_page::styles::init_once(); }

void apply_content(lv_obj_t *obj) { list_page::styles::apply_content(obj); }

void apply_list(lv_obj_t *obj) { list_page::styles::apply_list(obj); }

void apply_list_row(lv_obj_t *obj) { list_page::styles::apply_list_row(obj); }

void apply_list_label_left(lv_obj_t *obj) {
  list_page::styles::apply_list_label_left(obj);
  if (!obj) {
    return;
  }
  lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
  lv_obj_set_width(obj, LV_PCT(30));
}

void apply_list_label_right(lv_obj_t *obj) {
  list_page::styles::apply_list_label_right(obj);
  if (!obj) {
    return;
  }
  lv_label_set_long_mode(obj, LV_LABEL_LONG_DOT);
  lv_obj_set_width(obj, LV_PCT(65));
  lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
}

void apply_checked_state(lv_obj_t *obj) {
  list_page::styles::apply_checked_state(obj);
}

void clear_checked_state(lv_obj_t *obj) {
  list_page::styles::clear_checked_state(obj);
}

} // namespace lofi::ui::screens::about::styles
