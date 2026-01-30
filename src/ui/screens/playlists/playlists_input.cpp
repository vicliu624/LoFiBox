#include "ui/screens/playlists/playlists_input.h"

#include "ui/screens/list_page/list_page_input.h"

namespace lofi::ui::screens::playlists::input
{
void attach_row(UiScreen& screen, RowMeta& meta)
{
    list_page::input::attach_row(screen, meta);
}

void focus_first(lv_group_t* group, lv_obj_t* first)
{
    list_page::input::focus_first(group, first);
}

} // namespace lofi::ui::screens::playlists::input
