#pragma once

#include <lvgl.h>

void set_default_group(lv_group_t* group);
void ui_format_battery(int level, bool charging, char* out, size_t out_len);
