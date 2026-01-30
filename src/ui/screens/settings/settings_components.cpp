#include "ui/screens/settings/settings_components.h"

#include "ui/assets/assets.h"

namespace lofi::ui::screens::settings
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    const char* shuffle_state = (screen.player && screen.player->mode == app::PlaybackMode::Shuffle) ? "On" : "Off";
    const char* repeat_state = (screen.player && screen.player->mode == app::PlaybackMode::RepeatOne) ? "One" : "Off";
    components::add_item(screen, "Shuffle", shuffle_state, UiIntentKind::ToggleShuffle, PageId::None);
    components::add_item(screen, "Repeat", repeat_state, UiIntentKind::ToggleRepeat, PageId::None);
    components::add_item(screen, "EQ", "Flat", UiIntentKind::OpenEq, PageId::Eq, 0, 0, &Equalizer);
    components::add_item(screen, "Backlight", "10s", UiIntentKind::None, PageId::None);
    components::add_item(screen, "Sleep", "2m", UiIntentKind::None, PageId::None);
    components::add_item(screen, "Language", "EN", UiIntentKind::None, PageId::None);
    components::add_item(screen, "About", ">", UiIntentKind::OpenAbout, PageId::About, 0, 0, &About);
}

} // namespace lofi::ui::screens::settings
