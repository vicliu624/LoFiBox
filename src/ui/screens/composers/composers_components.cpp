#include "ui/screens/composers/composers_components.h"
#include "ui/common/sort_utils.h"

namespace lofi::ui::screens::composers
{
void populate(UiScreen& screen)
{
    components::reset_items(screen);
    if (!screen.library || screen.library->composer_count == 0) {
        components::add_item(screen, "No Composers", nullptr, UiIntentKind::None, PageId::None);
        return;
    }

    int idx[app::kMaxComposers];
    for (int i = 0; i < screen.library->composer_count; ++i) {
        idx[i] = i;
    }
    sort::string_indices(screen.library->composers, idx, screen.library->composer_count);

    for (int i = 0; i < screen.library->composer_count; ++i) {
        components::add_item(screen, screen.library->composers[idx[i]], "", UiIntentKind::OpenComposer, PageId::Songs, idx[i]);
    }
}

} // namespace lofi::ui::screens::composers
