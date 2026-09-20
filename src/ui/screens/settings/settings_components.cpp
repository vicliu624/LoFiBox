#include "ui/screens/settings/settings_components.h"

#include "ui/LV_Helper.h"
#include "app/music_lights.h"
#include "app/network.h"
#include "board/BoardBase.h"
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
  if (screen.player) {
    char volume[16] = {0};
    snprintf(volume, sizeof(volume), "%u / 21",
             static_cast<unsigned>(app::player_get_volume(*screen.player)));
    components::add_item(screen, "Volume", volume, UiIntentKind::CycleVolume,
                         PageId::None);
  }
  if (board.supportsAudioOutputSelection() && screen.player) {
    screen.state.last_audio_output_version = screen.player->audio_output_version;
    components::add_item(screen, "Audio output",
                         app::player_audio_output_label(*screen.player),
                         UiIntentKind::OpenAudioOutputSettings,
                         PageId::AudioOutputSettings);
  }
  if (board.supportsMusicLights()) {
    components::add_item(screen, "Bottom3 Music LEDs",
                         app::music_lights::enabled() ? "On" : "Off",
                         UiIntentKind::ToggleMusicLights, PageId::None);
  }
  components::add_item(screen, "Wi-Fi", app::network::status_label(),
                       UiIntentKind::OpenWifiSettings, PageId::WifiSettings);
}

void populate_audio_output(UiScreen &screen) {
  components::reset_items(screen);
  if (!screen.player) {
    return;
  }
  screen.state.last_audio_output_version = screen.player->audio_output_version;
  const app::AudioOutputMode current = screen.player->audio_output_mode;
  const char *active = screen.player->headphones_active ? "Headphones"
                                                         : "Core2 speaker";
  components::add_item(screen, "Auto", String("Now: ") + active,
                       UiIntentKind::SetAudioOutputMode, PageId::None,
                       static_cast<int>(app::AudioOutputMode::Auto));
  components::add_item(screen, "Core2 speaker",
                       current == app::AudioOutputMode::Speaker ? "Selected" : "",
                       UiIntentKind::SetAudioOutputMode, PageId::None,
                       static_cast<int>(app::AudioOutputMode::Speaker));
  components::add_item(screen, "Module Audio headphones",
                       current == app::AudioOutputMode::Headphones ? "Selected" : "",
                       UiIntentKind::SetAudioOutputMode, PageId::None,
                       static_cast<int>(app::AudioOutputMode::Headphones));
}

void populate_wifi(UiScreen &screen) {
  components::reset_items(screen);
  components::add_item(screen, "Wi-Fi", app::network::enabled() ? "On" : "Off",
                       UiIntentKind::ToggleWifi, PageId::None);
  if (!app::network::enabled()) {
    components::add_item(screen, "Enable Wi-Fi to scan", "",
                         UiIntentKind::None, PageId::None);
    return;
  }

  components::add_item(screen, "Scan networks",
                       app::network::scan_in_progress() ? "Scanning..." : "",
                       UiIntentKind::ScanWifi, PageId::None);
  const String current = app::network::saved_ssid();
  if (!current.isEmpty()) {
    components::add_item(screen, "Saved network", current,
                         UiIntentKind::EditWifiSsid, PageId::None);
  }
  for (uint8_t i = 0; i < app::network::scan_count(); ++i) {
    const app::network::ScanResult *result = app::network::scan_result(i);
    if (!result) continue;
    String detail = String(result->rssi) + " dBm" +
                    (result->secured ? "  Lock" : "  Open");
    components::add_item(screen, result->ssid, detail,
                         UiIntentKind::SelectWifiNetwork, PageId::None, i);
  }
  components::add_item(screen, "Other network", "Enter SSID",
                       UiIntentKind::EditWifiSsid, PageId::None);
}

} // namespace lofi::ui::screens::settings
