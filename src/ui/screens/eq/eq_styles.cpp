#include "ui/screens/eq/eq_styles.h"

#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::eq::styles
{
namespace
{
static bool s_inited = false;
static lv_style_t s_content;
static lv_style_t s_panel;
static lv_style_t s_graph;
static lv_style_t s_db_label;
static lv_style_t s_slider;
static lv_style_t s_slider_indicator;
static lv_style_t s_slider_knob;
static lv_style_t s_slider_sel;
static lv_style_t s_label;
static lv_style_t s_preset;
static lv_style_t s_preset_value;
}

void init_once()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    const lv_font_t* font = font_noto_sc_16();

    lv_style_init(&s_content);
    lv_style_set_bg_color(&s_content, lv_color_hex(0x0b0b0b));
    lv_style_set_bg_opa(&s_content, LV_OPA_COVER);
    lv_style_set_bg_grad_dir(&s_content, LV_GRAD_DIR_NONE);
    lv_style_set_pad_all(&s_content, 0);
    lv_style_set_radius(&s_content, 0);
    lv_style_set_border_width(&s_content, 0);
    lv_style_set_border_opa(&s_content, LV_OPA_0);
    lv_style_set_outline_width(&s_content, 0);
    lv_style_set_outline_opa(&s_content, LV_OPA_0);

    lv_style_init(&s_panel);
    lv_style_set_bg_color(&s_panel, lv_color_hex(0x0d0e10));
    lv_style_set_bg_grad_color(&s_panel, lv_color_hex(0x0b0b0c));
    lv_style_set_bg_grad_dir(&s_panel, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&s_panel, LV_OPA_COVER);
    lv_style_set_border_width(&s_panel, 0);
    lv_style_set_outline_width(&s_panel, 0);
    lv_style_set_pad_all(&s_panel, 0);

    lv_style_init(&s_graph);
    lv_style_set_bg_color(&s_graph, lv_color_hex(0x0b0b0b));
    lv_style_set_bg_grad_dir(&s_graph, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&s_graph, LV_OPA_0);
    lv_style_set_border_width(&s_graph, 0);
    lv_style_set_radius(&s_graph, 0);
    lv_style_set_pad_all(&s_graph, 0);

    lv_style_init(&s_db_label);
    lv_style_set_text_font(&s_db_label, font);
    lv_style_set_text_color(&s_db_label, lv_color_hex(0x8c939a));
    lv_style_set_text_opa(&s_db_label, LV_OPA_COVER);

    lv_style_init(&s_slider);
    lv_style_set_bg_color(&s_slider, lv_color_hex(0x1a1d21));
    lv_style_set_bg_grad_color(&s_slider, lv_color_hex(0x14161a));
    lv_style_set_bg_grad_dir(&s_slider, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&s_slider, LV_OPA_COVER);
    lv_style_set_radius(&s_slider, 6);
    lv_style_set_border_width(&s_slider, 0);

    lv_style_init(&s_slider_indicator);
    lv_style_set_bg_color(&s_slider_indicator, lv_color_hex(0xffb257));
    lv_style_set_bg_grad_color(&s_slider_indicator, lv_color_hex(0xff7f2a));
    lv_style_set_bg_grad_dir(&s_slider_indicator, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&s_slider_indicator, LV_OPA_COVER);
    lv_style_set_radius(&s_slider_indicator, 6);

    lv_style_init(&s_slider_knob);
    lv_style_set_bg_color(&s_slider_knob, lv_color_hex(0xffc36a));
    lv_style_set_bg_grad_color(&s_slider_knob, lv_color_hex(0xff8a38));
    lv_style_set_bg_grad_dir(&s_slider_knob, LV_GRAD_DIR_VER);
    lv_style_set_radius(&s_slider_knob, 6);
    lv_style_set_pad_all(&s_slider_knob, 3);
    lv_style_set_border_width(&s_slider_knob, 1);
    lv_style_set_border_color(&s_slider_knob, lv_color_hex(0xffe0b0));
    lv_style_set_shadow_width(&s_slider_knob, 4);
    lv_style_set_shadow_color(&s_slider_knob, lv_color_hex(0x3a1e00));
    lv_style_set_shadow_opa(&s_slider_knob, LV_OPA_30);

    lv_style_init(&s_slider_sel);
    lv_style_set_bg_color(&s_slider_sel, lv_color_hex(0x5aa9ff));
    lv_style_set_bg_opa(&s_slider_sel, LV_OPA_COVER);

    lv_style_init(&s_label);
    lv_style_set_text_font(&s_label, font);
    lv_style_set_text_color(&s_label, lv_color_hex(0xcfd3d7));
    lv_style_set_text_opa(&s_label, LV_OPA_COVER);

    lv_style_init(&s_preset);
    lv_style_set_text_font(&s_preset, font);
    lv_style_set_text_color(&s_preset, lv_color_hex(0xaab0b6));
    lv_style_set_text_opa(&s_preset, LV_OPA_COVER);

    lv_style_init(&s_preset_value);
    lv_style_set_text_font(&s_preset_value, font);
    lv_style_set_text_color(&s_preset_value, lv_color_hex(0xffffff));
    lv_style_set_text_opa(&s_preset_value, LV_OPA_COVER);
}

void apply_content(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_content, LV_PART_MAIN);
}

void apply_panel(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_panel, LV_PART_MAIN);
}

void apply_graph(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_graph, LV_PART_MAIN);
}

void apply_db_label(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_db_label, LV_PART_MAIN);
    lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
}

void apply_slider(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_slider, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_slider_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &s_slider_knob, LV_PART_KNOB);
}

void apply_slider_selected(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_slider_sel, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_outline_width(obj, 1, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_outline_color(obj, lv_color_hex(0x67c2ff), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_outline_opa(obj, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x55b7ff), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x2f75ff), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x7fd1ff), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_color(obj, lv_color_hex(0x2f86ff), LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_grad_dir(obj, LV_GRAD_DIR_VER, LV_PART_KNOB | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_CHECKED);
}

void apply_label(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(obj, 0, LV_PART_MAIN);
}

void apply_preset(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_preset, LV_PART_MAIN);
}

void apply_preset_value(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_preset_value, LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj, 6, LV_PART_MAIN);
}

} // namespace lofi::ui::screens::eq::styles
