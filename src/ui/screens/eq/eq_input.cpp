#include "ui/screens/eq/eq_input.h"

#include "app/eq_dsp.h"
#include "ui/screens/eq/eq_components.h"

namespace lofi::ui::screens::eq::input {
namespace {
constexpr int kVisibleBands = 6;
const int kBandMap[kVisibleBands] = {0, 1, 2, 3, 4, 5};

void handle_key(UiScreen &screen, uint32_t key) {
  if (key == LV_KEY_ENTER) {
    screen.state.eq_editing = !screen.state.eq_editing;
    update(screen);
    return;
  }
  if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
    if (screen.state.eq_editing) {
      screen.state.eq_editing = false;
      update(screen);
      return;
    }
    UiIntent intent{};
    intent.kind = UiIntentKind::NavigateBack;
    request_intent(&screen, intent);
    return;
  }

  if (screen.state.eq_editing) {
    int delta = 0;
    if (key == LV_KEY_UP || key == LV_KEY_RIGHT || key == LV_KEY_PREV) {
      delta = 1;
    } else if (key == LV_KEY_DOWN || key == LV_KEY_LEFT || key == LV_KEY_NEXT) {
      delta = -1;
    }
    if (delta != 0) {
      int slot = screen.state.eq_selected_band;
      int band = (slot >= 0 && slot < kVisibleBands) ? kBandMap[slot] : 0;
      int8_t db = app::eq::get_band(band);
      app::eq::set_band(band, static_cast<int8_t>(db + delta));
      update(screen);
    }
    return;
  }

  int step = 0;
  if (key == LV_KEY_UP || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
    step = -1;
  } else if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT || key == LV_KEY_NEXT) {
    step = 1;
  }
  if (step != 0) {
    screen.state.eq_selected_band += step;
    if (screen.state.eq_selected_band < 0) {
      screen.state.eq_selected_band = 0;
    }
    if (screen.state.eq_selected_band >= kVisibleBands) {
      screen.state.eq_selected_band = kVisibleBands - 1;
    }
    update(screen);
    return;
  }
}

void key_event_cb(lv_event_t *e) {
  auto *screen = static_cast<UiScreen *>(lv_event_get_user_data(e));
  if (!screen_alive(screen)) {
    return;
  }
  if (lv_event_get_code(e) != LV_EVENT_KEY) {
    return;
  }
  handle_key(*screen, lv_event_get_key(e));
}
} // namespace

void attach(UiScreen &screen, lv_obj_t *key_sink) {
  if (!screen.group || !key_sink) {
    return;
  }
  lv_obj_add_event_cb(key_sink, key_event_cb, LV_EVENT_KEY, &screen);
  lv_group_add_obj(screen.group, key_sink);
  lv_group_focus_obj(key_sink);
}

} // namespace lofi::ui::screens::eq::input
