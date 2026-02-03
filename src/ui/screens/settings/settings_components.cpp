#include "ui/screens/settings/settings_components.h"

#include "ui/LV_Helper.h"
#include "ui/assets/assets.h"

namespace lofi::ui::screens::settings {
void populate(UiScreen &screen) {
  components::reset_items(screen);
  const char *shuffle_state =
      (screen.player && screen.player->mode == app::PlaybackMode::Shuffle)
          ? "On"
          : "Off";
  const char *repeat_state =
      (screen.player && screen.player->mode == app::PlaybackMode::RepeatOne)
          ? "One"
          : "Off";
  components::add_item(screen, "Shuffle", shuffle_state,
                       UiIntentKind::ToggleShuffle, PageId::None);
  components::add_item(screen, "Repeat", repeat_state,
                       UiIntentKind::ToggleRepeat, PageId::None);
  char backlight[16] = {0};
  char sleep[16] = {0};
  lvHelperFormatTimeout(backlight, sizeof(backlight),
                        lvHelperGetBacklightTimeoutMs());
  lvHelperFormatTimeout(sleep, sizeof(sleep), lvHelperGetSleepTimeoutMs());
  components::add_item(screen, "Backlight", backlight,
                       UiIntentKind::CycleBacklightTimeout, PageId::None);
  components::add_item(screen, "Sleep", sleep, UiIntentKind::CycleSleepTimeout,
                       PageId::None);
}

} // namespace lofi::ui::screens::settings
