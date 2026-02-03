#pragma once

#include "app/library.h"

namespace lofi::ui::sort {
int compare_ci(const char *a, const char *b);
void string_indices(const char *const *arr, int *idx, int count);
void album_indices(const app::Library &lib, int *idx, int count);
void track_indices_by_title(const app::Library &lib, int *idx, int count);
void tracks_by_added(const app::Library &lib, int *idx, int count);
void tracks_by_play_count(const app::Library &lib, int *idx, int count);
void tracks_by_last_played(const app::Library &lib, int *idx, int count);
} // namespace lofi::ui::sort
