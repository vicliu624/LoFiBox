#include "ui/screens/list_page/list_page_input.h"

#include "ui/screens/list_page/list_page_layout.h"
#include "ui/screens/list_page/list_page_styles.h"

namespace lofi::ui::screens::list_page::input {
namespace {
void apply_item_to_row(UiScreen &screen, RowMeta &meta, int item_index) {
  meta.item_index = item_index;
  if (item_index < 0 || item_index >= screen.items_count) {
    meta.intent = {};
    if (meta.row) {
      lv_obj_add_flag(meta.row, LV_OBJ_FLAG_HIDDEN);
    }
    if (meta.left_label) {
      lv_label_set_text(meta.left_label, "");
    }
    if (meta.right_label) {
      lv_label_set_text(meta.right_label, "");
    }
    return;
  }

  if (meta.row) {
    lv_obj_clear_flag(meta.row, LV_OBJ_FLAG_HIDDEN);
  }

  ListItem &item = screen.items[item_index];
  meta.intent.kind = item.action;
  meta.intent.next = item.next;
  meta.intent.value = item.value;
  meta.intent.value2 = item.value2;
  meta.intent.item = &item;

  if (meta.icon) {
    bool show_icon = (item.icon != nullptr);
    list_page::layout::ListRowLayout row_layout{};
    row_layout.row = meta.row;
    row_layout.icon = meta.icon;
    row_layout.left_label = meta.left_label;
    row_layout.right_label = meta.right_label;
    list_page::layout::set_row_icon_visible(row_layout, show_icon);
    if (show_icon) {
      lv_img_set_src(meta.icon, item.icon);
      lv_obj_clear_flag(meta.icon, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(meta.icon, LV_OBJ_FLAG_HIDDEN);
    }
  }

  if (meta.left_label) {
    lv_label_set_text(meta.left_label, item.left.c_str());
  }

  const char *right_text =
      item.right.length() > 0 ? item.right.c_str() : nullptr;
  if (!right_text) {
    switch (item.action) {
    case UiIntentKind::Navigate:
    case UiIntentKind::OpenAlbumsAll:
    case UiIntentKind::OpenSongsAll:
    case UiIntentKind::OpenArtist:
    case UiIntentKind::OpenAlbum:
    case UiIntentKind::OpenGenre:
    case UiIntentKind::OpenComposer:
    case UiIntentKind::OpenPlaylist:
    case UiIntentKind::OpenEq:
    case UiIntentKind::OpenAbout:
      right_text = LV_SYMBOL_RIGHT;
      break;
    default:
      break;
    }
  }
  if (meta.right_label) {
    lv_label_set_text(meta.right_label, right_text ? right_text : "");
  }
}

int visible_rows(const UiScreen &screen) {
  return screen.row_count > 0 ? screen.row_count : 0;
}

void focus_row_for_selection(UiScreen &screen) {
  if (!screen.group) {
    return;
  }
  int visible = visible_rows(screen);
  if (visible <= 0) {
    return;
  }
  int row_index = screen.state.list_selected - screen.state.list_offset;
  if (row_index < 0) {
    row_index = 0;
  } else if (row_index >= visible) {
    row_index = visible - 1;
  }
  if (screen.rows[row_index].row) {
    lv_group_focus_obj(screen.rows[row_index].row);
  }
}

void row_event_cb(lv_event_t *e) {
  auto *meta = static_cast<RowMeta *>(lv_event_get_user_data(e));
  if (!meta || !meta->screen) {
    return;
  }
  UiScreen *screen = meta->screen;
  if (!screen_alive(screen)) {
    return;
  }

  lv_obj_t *row = static_cast<lv_obj_t *>(lv_event_get_target(e));
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_FOCUSED) {
    if (meta->item_index >= 0) {
      screen->state.list_selected = meta->item_index;
    }
    styles::apply_checked_state(row);
    if (meta->left_label) {
      styles::apply_checked_state(meta->left_label);
    }
    if (meta->right_label) {
      styles::apply_checked_state(meta->right_label);
    }
    if (screen->state.current == PageId::About) {
      lv_obj_scroll_to_view(row, LV_ANIM_OFF);
    }
    return;
  }

  if (code == LV_EVENT_DEFOCUSED) {
    styles::clear_checked_state(row);
    if (meta->left_label) {
      styles::clear_checked_state(meta->left_label);
    }
    if (meta->right_label) {
      styles::clear_checked_state(meta->right_label);
    }
    return;
  }

  if (code == LV_EVENT_CLICKED) {
    request_intent(screen, meta->intent);
    return;
  }

  if (code == LV_EVENT_KEY) {
    uint32_t key = lv_event_get_key(e);
    if (screen->state.current == PageId::About) {
      if (key == LV_KEY_UP || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
        if (meta->item_index <= 0) {
          return;
        }
        lv_group_focus_prev(screen->group);
        return;
      }
      if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT || key == LV_KEY_NEXT) {
        if (meta->item_index >= screen->items_count - 1) {
          return;
        }
        lv_group_focus_next(screen->group);
        return;
      }
      if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
        UiIntent intent{};
        intent.kind = UiIntentKind::NavigateBack;
        request_intent(screen, intent);
        return;
      }
    }
    if (key == LV_KEY_UP || key == LV_KEY_LEFT || key == LV_KEY_PREV) {
      move_selection(*screen, -1);
      return;
    }
    if (key == LV_KEY_DOWN || key == LV_KEY_RIGHT || key == LV_KEY_NEXT) {
      move_selection(*screen, 1);
      return;
    }
    if (key == LV_KEY_ESC || key == LV_KEY_BACKSPACE) {
      UiIntent intent{};
      intent.kind = UiIntentKind::NavigateBack;
      request_intent(screen, intent);
    }
  }
}
} // namespace

void attach_row(UiScreen &screen, RowMeta &meta) {
  if (!screen.group || !meta.row) {
    return;
  }
  meta.screen = &screen;
  lv_obj_add_event_cb(meta.row, row_event_cb, LV_EVENT_ALL, &meta);
  lv_group_add_obj(screen.group, meta.row);
}

void focus_first(lv_group_t *group, lv_obj_t *first) {
  if (!group || !first) {
    return;
  }
  lv_group_focus_obj(first);
}

void refresh_rows(UiScreen &screen) {
  int visible = visible_rows(screen);
  if (visible <= 0) {
    return;
  }
  int offset = screen.state.list_offset;
  for (int i = 0; i < visible; ++i) {
    apply_item_to_row(screen, screen.rows[i], offset + i);
  }
  focus_row_for_selection(screen);
}

void move_selection(UiScreen &screen, int delta) {
  if (screen.items_count <= 0) {
    return;
  }
  int selected = screen.state.list_selected + delta;
  if (screen.state.current == PageId::About) {
    if (selected < 0) {
      selected = 0;
    } else if (selected >= screen.items_count) {
      selected = screen.items_count - 1;
    }
  } else {
    if (selected < 0) {
      selected = screen.items_count - 1;
    } else if (selected >= screen.items_count) {
      selected = 0;
    }
  }
  if (selected == screen.state.list_selected) {
    return;
  }
  screen.state.list_selected = selected;

  int visible = visible_rows(screen);
  if (visible <= 0) {
    return;
  }
  if (selected < screen.state.list_offset) {
    screen.state.list_offset = selected;
  } else if (selected >= screen.state.list_offset + visible) {
    screen.state.list_offset = selected - visible + 1;
  }

  refresh_rows(screen);
}

} // namespace lofi::ui::screens::list_page::input
