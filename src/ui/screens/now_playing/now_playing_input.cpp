#include "ui/screens/now_playing/now_playing_input.h"

namespace lofi::ui::screens::now_playing::input
{
namespace
{
void key_cb(lv_event_t* e)
{
    auto* screen = static_cast<UiScreen*>(lv_event_get_user_data(e));
    if (!screen_alive(screen)) {
        return;
    }
    if (lv_event_get_code(e) != LV_EVENT_KEY) {
        return;
    }

    uint32_t key = lv_event_get_key(e);
    UiIntent intent{};
    if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
        intent.kind = UiIntentKind::NavigateBack;
    } else if (key == LV_KEY_LEFT) {
        intent.kind = UiIntentKind::PrevTrack;
    } else if (key == LV_KEY_RIGHT) {
        intent.kind = UiIntentKind::NextTrack;
    } else if (key == LV_KEY_ENTER) {
        intent.kind = UiIntentKind::TogglePause;
    } else {
        return;
    }

    request_intent(screen, intent);
}
} // namespace

void attach(UiScreen& screen, lv_obj_t* key_sink)
{
    if (!screen.group || !key_sink) {
        return;
    }
    lv_obj_add_event_cb(key_sink, key_cb, LV_EVENT_KEY, &screen);
    lv_group_add_obj(screen.group, key_sink);
    lv_group_focus_obj(key_sink);
}

} // namespace lofi::ui::screens::now_playing::input
