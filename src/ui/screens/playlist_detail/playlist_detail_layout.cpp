#include "ui/screens/playlist_detail/playlist_detail_layout.h"

#include "ui/screens/list_page/list_page_layout.h"

// Wireframe (playlist detail)
// +----------------------------------------------+
// | < Back              TITLE               BAT  |  topbar
// +----------------------------------------------+
// |  Row 1                                        |
// |  Row 2                                        |
// |  Row 3                                        |
// |  Row 4                                        |
// |  ...                                          |
// +----------------------------------------------+
//
// Tree (playlist detail)
// content
// +- list
//    +- row
//    |  +- left_label
//    |  +- right_label
//    +- row
//    +- ...

namespace lofi::ui::screens::playlist_detail::layout {
lv_coord_t row_height() { return list_page::layout::row_height(); }

ListLayout create_list(lv_obj_t *content) {
  return list_page::layout::create_list(content);
}

ListRowLayout create_list_row(lv_obj_t *list) {
  return list_page::layout::create_list_row(list);
}

} // namespace lofi::ui::screens::playlist_detail::layout
