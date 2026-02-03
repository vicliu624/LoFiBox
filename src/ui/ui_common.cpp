#include "ui_common.h"

#include <cstdio>

void set_default_group(lv_group_t *group) {
  if (!group) {
    return;
  }
  lv_group_set_editing(group, false);
  lv_group_set_wrap(group, true);
  lv_group_set_refocus_policy(group, LV_GROUP_REFOCUS_POLICY_NEXT);
  lv_indev_t *cur_drv = nullptr;
  for (;;) {
    cur_drv = lv_indev_get_next(cur_drv);
    if (!cur_drv) {
      break;
    }
    if (lv_indev_get_type(cur_drv) == LV_INDEV_TYPE_KEYPAD) {
      lv_indev_set_group(cur_drv, group);
    }
    if (lv_indev_get_type(cur_drv) == LV_INDEV_TYPE_ENCODER) {
      lv_indev_set_group(cur_drv, group);
    }
    if (lv_indev_get_type(cur_drv) == LV_INDEV_TYPE_POINTER) {
      lv_indev_set_group(cur_drv, group);
    }
  }
  lv_group_set_default(group);
}

void ui_format_battery(int level, bool charging, char *out, size_t out_len) {
  if (!out || out_len == 0) {
    return;
  }
  const char *icon = LV_SYMBOL_BATTERY_EMPTY;
  if (level >= 80) {
    icon = LV_SYMBOL_BATTERY_FULL;
  } else if (level >= 60) {
    icon = LV_SYMBOL_BATTERY_3;
  } else if (level >= 40) {
    icon = LV_SYMBOL_BATTERY_2;
  } else if (level >= 20) {
    icon = LV_SYMBOL_BATTERY_1;
  }

  if (charging) {
    snprintf(out, out_len, "%s%s", LV_SYMBOL_CHARGE, icon);
  } else {
    snprintf(out, out_len, "%s", icon);
  }
}
