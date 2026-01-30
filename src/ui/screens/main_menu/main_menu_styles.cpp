#include "ui/screens/main_menu/main_menu_styles.h"

namespace lofi::ui::screens::main_menu::styles
{
namespace
{
static bool s_inited = false;
static lv_style_t s_content;
static lv_style_t s_item;
static lv_style_t s_item_checked;
static lv_style_t s_label;
static lv_style_t s_label_checked;
static lv_style_t s_arrow;
static lv_style_t s_dot;
static lv_style_t s_dot_active;
}

void init_once()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    lv_coord_t w = lv_display_get_horizontal_resolution(nullptr);
    const lv_font_t* label_font = (w >= 360) ? &lv_font_montserrat_18 : &lv_font_montserrat_14;
    const lv_font_t* arrow_font = (w >= 360) ? &lv_font_montserrat_18 : &lv_font_montserrat_14;

    lv_style_init(&s_content);
    lv_style_set_bg_color(&s_content, lv_color_hex(0x0b0b0b));
    lv_style_set_bg_opa(&s_content, LV_OPA_COVER);
    lv_style_set_bg_grad_color(&s_content, lv_color_hex(0x0b0b0b));
    lv_style_set_bg_grad_dir(&s_content, LV_GRAD_DIR_NONE);
    lv_style_set_pad_all(&s_content, 0);
    lv_style_set_radius(&s_content, 0);
    lv_style_set_border_width(&s_content, 0);
    lv_style_set_border_opa(&s_content, LV_OPA_0);
    lv_style_set_outline_width(&s_content, 0);
    lv_style_set_outline_opa(&s_content, LV_OPA_0);

    lv_style_init(&s_item);
    lv_style_set_bg_opa(&s_item, LV_OPA_0);
    lv_style_set_border_width(&s_item, 0);
    lv_style_set_shadow_width(&s_item, 0);
    lv_style_set_pad_all(&s_item, 0);
    lv_style_set_radius(&s_item, 0);
    lv_style_set_outline_width(&s_item, 0);
    lv_style_set_outline_opa(&s_item, LV_OPA_0);
    lv_style_set_border_opa(&s_item, LV_OPA_0);

    lv_style_init(&s_item_checked);
    lv_style_set_outline_width(&s_item_checked, 0);
    lv_style_set_outline_color(&s_item_checked, lv_color_hex(0x5fb0ff));
    lv_style_set_outline_pad(&s_item_checked, 0);
    lv_style_set_radius(&s_item_checked, 0);
    lv_style_set_outline_opa(&s_item_checked, LV_OPA_0);

    lv_style_init(&s_label);
    lv_style_set_text_font(&s_label, label_font);
    lv_style_set_text_color(&s_label, lv_color_hex(0xf2f2f2));
    lv_style_set_text_align(&s_label, LV_TEXT_ALIGN_CENTER);

    lv_style_init(&s_label_checked);
    lv_style_set_text_color(&s_label_checked, lv_color_hex(0xf2f2f2));

    lv_style_init(&s_arrow);
    lv_style_set_text_font(&s_arrow, arrow_font);
    lv_style_set_text_color(&s_arrow, lv_color_hex(0xbdbdbd));

    lv_style_init(&s_dot);
    lv_style_set_radius(&s_dot, 0);
    lv_style_set_bg_color(&s_dot, lv_color_hex(0x2c1250));
    lv_style_set_bg_opa(&s_dot, LV_OPA_COVER);
    lv_style_set_border_width(&s_dot, 0);

    lv_style_init(&s_dot_active);
    lv_style_set_bg_color(&s_dot_active, lv_color_hex(0x0a5cff));
}

void apply_content(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_content, LV_PART_MAIN);
}

void apply_item(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_item, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_item, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, &s_item, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &s_item, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &s_item, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_label(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &s_label, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_arrow(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_arrow, LV_PART_MAIN);
}

void apply_dot(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_dot, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_dot_active, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_checked_state(lv_obj_t* obj)
{
    lv_obj_add_state(obj, LV_STATE_CHECKED);
}

void clear_checked_state(lv_obj_t* obj)
{
    lv_obj_clear_state(obj, LV_STATE_CHECKED);
}

} // namespace lofi::ui::screens::main_menu::styles
