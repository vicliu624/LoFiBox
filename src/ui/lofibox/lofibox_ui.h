#pragma once

#include "app/library.h"
#include "app/player.h"

namespace lofi::ui {
enum class MediaKey {
  PlayPause = 0,
  Next,
  Prev,
};

enum class GlobalKey {
  VolumeCycle = 0,
  PlaybackModeCycle,
  DeletePrompt,
  DeleteConfirm,
  DeleteCancel,
};

void init(app::Library *library, app::PlayerState *player);
void tick();
void handle_media_key(MediaKey key);
void handle_global_key(GlobalKey key);
void rebuild();

} // namespace lofi::ui
