#include "ui/screens/genres/genres_styles.h"

#include "ui/screens/list_page/list_page_styles.h"

namespace lofi::ui::screens::genres::styles {
void init_once() { list_page::styles::init_once(); }

void apply_content(lv_obj_t *obj) { list_page::styles::apply_content(obj); }

void apply_list(lv_obj_t *obj) { list_page::styles::apply_list(obj); }

void apply_list_row(lv_obj_t *obj) { list_page::styles::apply_list_row(obj); }

void apply_list_label_left(lv_obj_t *obj) {
  list_page::styles::apply_list_label_left(obj);
}

void apply_list_label_right(lv_obj_t *obj) {
  list_page::styles::apply_list_label_right(obj);
}

void apply_checked_state(lv_obj_t *obj) {
  list_page::styles::apply_checked_state(obj);
}

void clear_checked_state(lv_obj_t *obj) {
  list_page::styles::clear_checked_state(obj);
}

} // namespace lofi::ui::screens::genres::styles
