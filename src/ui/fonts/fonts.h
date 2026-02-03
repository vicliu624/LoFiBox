#pragma once

#include "lvgl.h"

LV_FONT_DECLARE(lv_font_noto_sc_16_2bpp);

void init_font_fallbacks();
const lv_font_t *font_noto_sc_16();
