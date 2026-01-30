#include "ui/screens/eq/eq_components.h"

namespace lofi::ui::screens::eq
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    components::add_item(screen, "Preset", "Flat", UiIntentKind::None, PageId::None);
    components::add_item(screen, "Preamp", "-6.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "31 Hz", "+0.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "62 Hz", "+1.5 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "125 Hz", "+3.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "250 Hz", "+2.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "500 Hz", "+0.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "1 kHz", "-1.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "2 kHz", "-2.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "4 kHz", "+0.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "8 kHz", "+1.0 dB", UiIntentKind::None, PageId::None);
    components::add_item(screen, "16 kHz", "+2.0 dB", UiIntentKind::None, PageId::None);
}

} // namespace lofi::ui::screens::eq
