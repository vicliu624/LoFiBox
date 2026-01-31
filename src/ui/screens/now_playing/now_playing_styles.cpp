#include "ui/screens/now_playing/now_playing_styles.h"

namespace lofi::ui::screens::now_playing::styles
{
namespace
{
static bool s_inited = false;
static lv_style_t s_content;
static lv_style_t s_cover;
static lv_style_t s_title;
static lv_style_t s_subtitle;
static lv_style_t s_time_label;
static lv_style_t s_bar_wrap;
static lv_style_t s_bar;
static lv_style_t s_bar_indic;
static lv_style_t s_knob;
static lv_style_t s_controls_row;
static lv_style_t s_control_icon;
static lv_style_t s_control_icon_active;
static lv_style_t s_key_sink;
}

void init_once()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    lv_style_init(&s_content);
    lv_style_set_bg_color(&s_content, lv_color_hex(0x1d1f22));
    lv_style_set_bg_grad_color(&s_content, lv_color_hex(0x0f1012));
    lv_style_set_bg_grad_dir(&s_content, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&s_content, LV_OPA_COVER);
    lv_style_set_border_width(&s_content, 0);
    lv_style_set_border_opa(&s_content, LV_OPA_0);
    lv_style_set_outline_width(&s_content, 0);
    lv_style_set_outline_opa(&s_content, LV_OPA_0);
    lv_style_set_shadow_width(&s_content, 0);
    lv_style_set_shadow_opa(&s_content, LV_OPA_0);
    lv_style_set_radius(&s_content, 0);
    lv_style_set_pad_all(&s_content, 0);

    lv_style_init(&s_cover);
    lv_style_set_bg_color(&s_cover, lv_color_hex(0x0a0b0e));
    lv_style_set_border_width(&s_cover, 1);
    lv_style_set_border_color(&s_cover, lv_color_hex(0x6c6f76));

    lv_style_init(&s_title);
    lv_style_set_text_font(&s_title, &lv_font_source_han_sans_sc_16_cjk);
    lv_style_set_text_color(&s_title, lv_color_hex(0xf8f8f8));

    lv_style_init(&s_subtitle);
    lv_style_set_text_font(&s_subtitle, &lv_font_source_han_sans_sc_16_cjk);
    lv_style_set_text_color(&s_subtitle, lv_color_hex(0xc7c7c7));

    lv_style_init(&s_time_label);
    lv_style_set_text_font(&s_time_label, &lv_font_montserrat_14);
    lv_style_set_text_color(&s_time_label, lv_color_hex(0xbdbdbd));

    lv_style_init(&s_bar_wrap);
    lv_style_set_bg_opa(&s_bar_wrap, LV_OPA_0);
    lv_style_set_border_width(&s_bar_wrap, 0);

    lv_style_init(&s_bar);
    lv_style_set_radius(&s_bar, 0);
    lv_style_set_bg_color(&s_bar, lv_color_hex(0x3a3a3a));
    lv_style_set_border_width(&s_bar, 1);
    lv_style_set_border_color(&s_bar, lv_color_hex(0xf2f2f2));

    lv_style_init(&s_bar_indic);
    lv_style_set_bg_color(&s_bar_indic, lv_color_hex(0x5fb0ff));
    lv_style_set_bg_grad_color(&s_bar_indic, lv_color_hex(0x2d6bff));
    lv_style_set_bg_grad_dir(&s_bar_indic, LV_GRAD_DIR_HOR);
    lv_style_set_radius(&s_bar_indic, 0);
    lv_style_set_border_width(&s_bar_indic, 0);

    lv_style_init(&s_knob);
    lv_style_set_bg_color(&s_knob, lv_color_hex(0xf5f5f5));
    lv_style_set_border_width(&s_knob, 0);
    lv_style_set_radius(&s_knob, 0);

    lv_style_init(&s_controls_row);
    lv_style_set_bg_opa(&s_controls_row, LV_OPA_0);
    lv_style_set_border_width(&s_controls_row, 0);

    lv_style_init(&s_control_icon);
    lv_style_set_text_font(&s_control_icon, &lv_font_montserrat_18);
    lv_style_set_text_color(&s_control_icon, lv_color_hex(0xe6e6e6));

    lv_style_init(&s_control_icon_active);
    lv_style_set_text_color(&s_control_icon_active, lv_color_hex(0x5fb0ff));

    lv_style_init(&s_key_sink);
    lv_style_set_bg_opa(&s_key_sink, LV_OPA_0);
    lv_style_set_border_width(&s_key_sink, 0);
}

void apply_content(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_content, LV_PART_MAIN);
}

void apply_cover(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_cover, LV_PART_MAIN);
}

void apply_title(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_title, LV_PART_MAIN);
}

void apply_subtitle(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_subtitle, LV_PART_MAIN);
}

void apply_time_label(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_time_label, LV_PART_MAIN);
}

void apply_bar_wrap(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_bar_wrap, LV_PART_MAIN);
}

void apply_bar(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_bar, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_bar_indic, LV_PART_INDICATOR);
}

void apply_knob(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_knob, LV_PART_MAIN);
}

void apply_controls_row(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_controls_row, LV_PART_MAIN);
}

void apply_control_icon(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_control_icon, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_control_icon_active, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_key_sink(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_key_sink, LV_PART_MAIN);
}

} // namespace lofi::ui::screens::now_playing::styles
