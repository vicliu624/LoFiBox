#pragma once

#include "ui/lofibox/lofibox_ui_internal.h"

namespace lofi::ui::components
{
struct NavCommand
{
    enum class Type
    {
        None = 0,
        NavigateTo,
        Back,
        Rebuild,
    };

    Type type = Type::None;
    PageId target = PageId::None;
};

void build_page(UiScreen& screen);
void update_topbar(UiScreen& screen);
void update_now_playing(UiScreen& screen);
NavCommand handle_intent(UiScreen& screen, const UiIntent& intent);
void reset_items(UiScreen& screen);

ListItem* add_item(UiScreen& screen, const char* left, const char* right, UiIntentKind action, PageId next,
                   int value = 0, int value2 = 0, const lv_image_dsc_t* icon = nullptr);
ListItem* add_item(UiScreen& screen, const String& left, const String& right, UiIntentKind action, PageId next,
                   int value = 0, int value2 = 0, const lv_image_dsc_t* icon = nullptr);

int compare_ci(const String& a, const String& b);
void sort_string_indices(const String* arr, int* idx, int count);
void sort_album_indices(const app::Library& lib, int* idx, int count);
void sort_track_indices_by_title(const app::Library& lib, int* idx, int count);
void sort_tracks_by_added(const app::Library& lib, int* idx, int count);
void sort_tracks_by_play_count(const app::Library& lib, int* idx, int count);
void sort_tracks_by_last_played(const app::Library& lib, int* idx, int count);

} // namespace lofi::ui::components
