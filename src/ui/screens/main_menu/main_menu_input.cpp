#include "ui/screens/main_menu/main_menu_input.h"

#include "ui/lofibox/lofibox_components.h"
#include "ui/screens/main_menu/main_menu_styles.h"

namespace lofi::ui::screens::main_menu::input
{
namespace
{
void row_event_cb(lv_event_t* e)
{
    auto* meta = static_cast<RowMeta*>(lv_event_get_user_data(e));
    if (!meta || !meta->screen) {
        return;
    }
    UiScreen* screen = meta->screen;
    if (!screen_alive(screen)) {
        return;
    }

    lv_obj_t* row = static_cast<lv_obj_t*>(lv_event_get_target(e));
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_DEFOCUSED) {
        return;
    }

    if (code == LV_EVENT_CLICKED) {
        request_intent(screen, meta->intent);
        return;
    }

    if (code == LV_EVENT_KEY) {
        uint32_t key = lv_event_get_key(e);
        if (key == LV_KEY_UP || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
            if (screen->items_count > 0) {
                screen->state.menu_index--;
                if (screen->state.menu_index < 0) {
                    screen->state.menu_index = screen->items_count - 1;
                }
                components::update_main_menu(*screen);
            }
            return;
        }
        if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT || key == LV_KEY_NEXT) {
            if (screen->items_count > 0) {
                screen->state.menu_index++;
                if (screen->state.menu_index >= screen->items_count) {
                    screen->state.menu_index = 0;
                }
                components::update_main_menu(*screen);
            }
            return;
        }
        if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
            UiIntent intent{};
            intent.kind = UiIntentKind::NavigateBack;
            request_intent(screen, intent);
        }
    }
}
} // namespace

void attach_row(UiScreen& screen, RowMeta& meta)
{
    if (!screen.group || !meta.row) {
        return;
    }
    meta.screen = &screen;
    lv_obj_add_event_cb(meta.row, row_event_cb, LV_EVENT_ALL, &meta);
    lv_group_add_obj(screen.group, meta.row);
}

void focus_first(lv_group_t* group, lv_obj_t* first)
{
    if (!group || !first) {
        return;
    }
    lv_group_focus_obj(first);
}

} // namespace lofi::ui::screens::main_menu::input
