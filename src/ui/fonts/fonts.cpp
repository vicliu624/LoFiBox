#include "ui/fonts/fonts.h"

namespace {
lv_font_t s_sc;
bool s_inited = false;
}

void init_font_fallbacks()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    // Copy const fonts into RAM so we can safely set fallback pointers.
    s_sc = lv_font_noto_sc_16_2bpp;
    s_sc.fallback = &lv_font_montserrat_16;
}

const lv_font_t* font_noto_sc_16()
{
    init_font_fallbacks();
    lv_font_glyph_dsc_t dsc{};
    if (!lv_font_get_glyph_dsc(&s_sc, &dsc, 'A', 0)) {
        return &lv_font_montserrat_16;
    }
    return &s_sc;
}
