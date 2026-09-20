#include "ui/screens/wifi/wifi_components.h"

#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::wifi {
namespace {
constexpr uint8_t kCharacterRows = 4;
constexpr uint8_t kColumns = 8;
constexpr char kLower[] = "abcdefghijklmnopqrstuvwxyz012345";
constexpr char kUpper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-_.@!?";
constexpr char kSymbols[] = "0123456789!@#$%^&*()-_=+[]{}<>";

struct EditorView {
  lv_obj_t *title = nullptr;
  lv_obj_t *value = nullptr;
  lv_obj_t *hint = nullptr;
  lv_obj_t *cells[5][kColumns] = {};
  lv_obj_t *labels[5][kColumns] = {};
  lv_obj_t *key_sink = nullptr;
};
EditorView s_view;

const char *keyset(uint8_t index) {
  switch (index % 3) {
  case 1:
    return kUpper;
  case 2:
    return kSymbols;
  default:
    return kLower;
  }
}

String &editing_value(UiScreen &screen) {
  return screen.state.wifi_editing_ssid ? screen.state.wifi_ssid
                                        : screen.state.wifi_password;
}

void style_text(lv_obj_t *object, lv_color_t color) {
  lv_obj_set_style_text_font(object, font_noto_sc_16(), LV_PART_MAIN);
  lv_obj_set_style_text_color(object, color, LV_PART_MAIN);
  lv_obj_set_style_text_align(object, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

void refresh(UiScreen &screen) {
  if (!s_view.value)
    return;
  const bool ssid = screen.state.wifi_editing_ssid;
  String display = editing_value(screen);
  if (!ssid) {
    display = String('*').substring(0, 0);
    for (size_t i = 0; i < screen.state.wifi_password.length(); ++i) {
      display += '*';
    }
  }
  if (display.isEmpty())
    display = ssid ? "Enter network name" : "Enter password (or leave empty)";
  lv_label_set_text(s_view.title,
                    ssid ? "Wi-Fi network name" : "Wi-Fi password");
  lv_label_set_text(s_view.value, display.c_str());
  lv_label_set_text(s_view.hint, ssid ? "Select chars, then NEXT or SAVE"
                                      : "Select chars, then SAVE to connect");

  const char *set = keyset(screen.state.wifi_keyset);
  for (uint8_t row = 0; row < kCharacterRows; ++row) {
    for (uint8_t col = 0; col < kColumns; ++col) {
      const uint8_t index = row * kColumns + col;
      lv_label_set_text_fmt(s_view.labels[row][col], "%c", set[index]);
    }
  }
  static constexpr const char *kActions[kColumns] = {
      "abc", "ABC", "123", "DEL", "CLR", "NEXT", "SAVE", "CANCEL"};
  for (uint8_t col = 0; col < kColumns; ++col) {
    lv_label_set_text(s_view.labels[kCharacterRows][col], kActions[col]);
  }
  for (uint8_t row = 0; row <= kCharacterRows; ++row) {
    for (uint8_t col = 0; col < kColumns; ++col) {
      const bool selected =
          row == screen.state.wifi_key_row && col == screen.state.wifi_key_col;
      lv_obj_set_style_bg_color(s_view.cells[row][col],
                                selected ? lv_color_hex(0x2f76d2)
                                         : lv_color_hex(0x1a1e24),
                                LV_PART_MAIN);
      lv_obj_set_style_border_color(s_view.cells[row][col],
                                    selected ? lv_color_hex(0xa9d0ff)
                                             : lv_color_hex(0x333941),
                                    LV_PART_MAIN);
    }
  }
}

void request_back(UiScreen *screen) {
  UiIntent intent{};
  intent.kind = UiIntentKind::NavigateBack;
  request_intent(screen, intent);
}

void activate(UiScreen &screen) {
  const uint8_t row = screen.state.wifi_key_row;
  const uint8_t col = screen.state.wifi_key_col;
  if (row < kCharacterRows) {
    editing_value(screen) +=
        keyset(screen.state.wifi_keyset)[row * kColumns + col];
    refresh(screen);
    return;
  }
  switch (col) {
  case 0:
    screen.state.wifi_keyset = 0;
    break;
  case 1:
    screen.state.wifi_keyset = 1;
    break;
  case 2:
    screen.state.wifi_keyset = 2;
    break;
  case 3:
    if (!editing_value(screen).isEmpty())
      editing_value(screen).remove(editing_value(screen).length() - 1);
    break;
  case 4:
    editing_value(screen) = "";
    break;
  case 5: {
    UiIntent intent{};
    intent.kind = UiIntentKind::EditWifiPassword;
    request_intent(&screen, intent);
    return;
  }
  case 6: {
    UiIntent intent{};
    intent.kind = UiIntentKind::CommitWifiCredentials;
    request_intent(&screen, intent);
    return;
  }
  case 7:
    request_back(&screen);
    return;
  default:
    break;
  }
  refresh(screen);
}

void key_cb(lv_event_t *event) {
  auto *screen = static_cast<UiScreen *>(lv_event_get_user_data(event));
  if (!screen_alive(screen) || lv_event_get_code(event) != LV_EVENT_KEY)
    return;
  const uint32_t key = lv_event_get_key(event);
  if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
    if (key == LV_KEY_BACKSPACE && !editing_value(*screen).isEmpty()) {
      editing_value(*screen).remove(editing_value(*screen).length() - 1);
      refresh(*screen);
    } else {
      request_back(screen);
    }
    return;
  }
  if (key >= 32 && key <= 126) {
    editing_value(*screen) += static_cast<char>(key);
    refresh(*screen);
    return;
  }
  if (key == LV_KEY_UP) {
    screen->state.wifi_key_row =
        (screen->state.wifi_key_row + kCharacterRows) % (kCharacterRows + 1);
  } else if (key == LV_KEY_DOWN) {
    screen->state.wifi_key_row =
        (screen->state.wifi_key_row + 1) % (kCharacterRows + 1);
  } else if (key == LV_KEY_LEFT) {
    screen->state.wifi_key_col =
        (screen->state.wifi_key_col + kColumns - 1) % kColumns;
  } else if (key == LV_KEY_RIGHT) {
    screen->state.wifi_key_col = (screen->state.wifi_key_col + 1) % kColumns;
  } else if (key == LV_KEY_ENTER) {
    activate(*screen);
    return;
  } else {
    return;
  }
  refresh(*screen);
}

void cell_tap_cb(lv_event_t *event) {
  if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    return;
  auto *screen = static_cast<UiScreen *>(lv_event_get_user_data(event));
  if (!screen_alive(screen))
    return;

  lv_obj_t *target = static_cast<lv_obj_t *>(lv_event_get_target(event));
  for (uint8_t row = 0; row <= kCharacterRows; ++row) {
    for (uint8_t col = 0; col < kColumns; ++col) {
      if (s_view.cells[row][col] != target)
        continue;
      screen->state.wifi_key_row = row;
      screen->state.wifi_key_col = col;
      activate(*screen);
      return;
    }
  }
}
} // namespace

void build_credentials(UiScreen &screen) {
  s_view = {};
  lv_obj_t *content = screen.view.root.content;
  if (!content)
    return;
  lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(content, lv_color_hex(0x0d0f12), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(content, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_pad_all(content, 4, LV_PART_MAIN);

  // Credential widgets are positioned immediately while the page is being
  // built.  The shell uses percentage-sized parents, so force the first LVGL
  // layout pass before reading their dimensions.  Without this, Core2 saw a
  // 0x0 content area and every virtual-keyboard key was placed off-screen.
  if (screen.view.root.root) {
    lv_obj_update_layout(screen.view.root.root);
  }
  lv_obj_update_layout(content);
  lv_coord_t width = lv_obj_get_width(content);
  lv_coord_t height = lv_obj_get_height(content);
  if (width <= 0) {
    width = lv_display_get_horizontal_resolution(nullptr);
  }
  if (height <= 0) {
    height = lv_display_get_vertical_resolution(nullptr) - 28;
  }
  const lv_coord_t input_y = 23;
  const lv_coord_t grid_y = 62;
  const lv_coord_t row_h = (height - grid_y - 3) / 5;
  const lv_coord_t cell_w = (width - 8) / kColumns;
  Serial.printf(
      "[WIFI UI] credentials ssid=%s field=%s content=%dx%d cell=%dx%d\n",
      screen.state.wifi_ssid.c_str(),
      screen.state.wifi_editing_ssid ? "ssid" : "password", width, height,
      cell_w - 2, row_h - 2);

  s_view.title = lv_label_create(content);
  lv_obj_set_pos(s_view.title, 4, 1);
  lv_obj_set_width(s_view.title, width - 8);
  style_text(s_view.title, lv_color_hex(0xbfc8d4));
  s_view.value = lv_label_create(content);
  lv_obj_set_pos(s_view.value, 4, input_y);
  lv_obj_set_size(s_view.value, width - 8, 22);
  lv_obj_set_style_bg_color(s_view.value, lv_color_hex(0x1a2029), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(s_view.value, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(s_view.value, lv_color_hex(0x4a617a),
                                LV_PART_MAIN);
  lv_obj_set_style_border_width(s_view.value, 1, LV_PART_MAIN);
  style_text(s_view.value, lv_color_hex(0xffffff));
  lv_label_set_long_mode(s_view.value, LV_LABEL_LONG_DOT);
  s_view.hint = lv_label_create(content);
  lv_obj_set_pos(s_view.hint, 4, 47);
  lv_obj_set_width(s_view.hint, width - 8);
  style_text(s_view.hint, lv_color_hex(0x77818e));

  for (uint8_t row = 0; row <= kCharacterRows; ++row) {
    for (uint8_t col = 0; col < kColumns; ++col) {
      lv_obj_t *cell = lv_obj_create(content);
      s_view.cells[row][col] = cell;
      lv_obj_set_pos(cell, 4 + col * cell_w, grid_y + row * row_h);
      lv_obj_set_size(cell, cell_w - 2, row_h - 2);
      lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
      lv_obj_add_flag(cell, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_style_pad_all(cell, 0, LV_PART_MAIN);
      lv_obj_set_style_border_width(cell, 1, LV_PART_MAIN);
      lv_obj_add_event_cb(cell, cell_tap_cb, LV_EVENT_CLICKED, &screen);
      lv_obj_t *label = lv_label_create(cell);
      s_view.labels[row][col] = label;
      lv_obj_set_size(label, LV_PCT(100), LV_PCT(100));
      lv_obj_center(label);
      style_text(label, lv_color_hex(0xe6ebf2));
    }
  }
  s_view.key_sink = lv_btn_create(content);
  lv_obj_set_size(s_view.key_sink, 1, 1);
  lv_obj_add_flag(s_view.key_sink, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(s_view.key_sink, key_cb, LV_EVENT_KEY, &screen);
  if (screen.group) {
    lv_group_add_obj(screen.group, s_view.key_sink);
    lv_group_focus_obj(s_view.key_sink);
  }
  refresh(screen);
}

} // namespace lofi::ui::screens::wifi
