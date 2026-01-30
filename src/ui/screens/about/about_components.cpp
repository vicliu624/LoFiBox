#include "ui/screens/about/about_components.h"

#include <SD.h>

#include "board/BoardBase.h"

namespace lofi::ui::screens::about
{
namespace
{
void format_storage(char* out, size_t len)
{
    if (!board.isSDReady()) {
        snprintf(out, len, "No SD");
        return;
    }
    uint64_t total = SD.totalBytes();
    uint64_t used = SD.usedBytes();
    if (total == 0) {
        total = SD.cardSize();
    }
    if (total == 0) {
        snprintf(out, len, "-- / --");
        return;
    }
    uint32_t total_mb = static_cast<uint32_t>(total / (1024 * 1024));
    uint32_t used_mb = static_cast<uint32_t>(used / (1024 * 1024));
    if (total_mb >= 1024) {
        uint32_t total_gb = total_mb / 1024;
        uint32_t used_gb = used_mb / 1024;
        snprintf(out, len, "%lu/%lu GB", static_cast<unsigned long>(used_gb),
                 static_cast<unsigned long>(total_gb));
        return;
    }
    snprintf(out, len, "%lu/%lu MB", static_cast<unsigned long>(used_mb),
             static_cast<unsigned long>(total_mb));
}
}

void populate(UiScreen& screen)
{
    components::reset_items(screen);
    char storage[32] = {0};
    format_storage(storage, sizeof(storage));
    components::add_item(screen, "Storage", storage, UiIntentKind::None, PageId::None);
    components::add_item(screen, "Version", "0.1.0", UiIntentKind::None, PageId::None);
}

} // namespace lofi::ui::screens::about
