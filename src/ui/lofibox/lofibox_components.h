#pragma once

#include "ui/lofibox/lofibox_ui_internal.h"

namespace lofi::ui::components {
struct NavCommand {
  enum class Type {
    None = 0,
    NavigateTo,
    Back,
    Rebuild,
  };

  Type type = Type::None;
  PageId target = PageId::None;
};

void build_page(UiScreen &screen);
void update_topbar(UiScreen &screen);
void update_now_playing(UiScreen &screen);
void update_main_menu(UiScreen &screen);
NavCommand handle_intent(UiScreen &screen, const UiIntent &intent);
void reset_items(UiScreen &screen);

ListItem *add_item(UiScreen &screen, const char *left, const char *right,
                   UiIntentKind action, PageId next, int value = 0,
                   int value2 = 0, const lv_image_dsc_t *icon = nullptr);
ListItem *add_item(UiScreen &screen, const String &left, const String &right,
                   UiIntentKind action, PageId next, int value = 0,
                   int value2 = 0, const lv_image_dsc_t *icon = nullptr);

} // namespace lofi::ui::components
