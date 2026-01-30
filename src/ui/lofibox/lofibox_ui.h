#pragma once

#include "app/library.h"
#include "app/player.h"

namespace lofi::ui
{
void init(app::Library* library, app::PlayerState* player);
void tick();

} // namespace lofi::ui
