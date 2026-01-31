#include "ui/screens/list_page/list_page_styles.h"
#include "ui/fonts/fonts.h"
#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::list_page::styles
{
namespace
{
static bool s_inited = false;
static lv_style_t s_content;
static lv_style_t s_list;
static lv_style_t s_row;
static lv_style_t s_row_checked;
static lv_style_t s_label_left;
static lv_style_t s_label_right;
static lv_style_t s_label_checked;
}

void init_once()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    lv_coord_t w = lv_display_get_horizontal_resolution(nullptr);
    const lv_font_t* font_main = font_noto_sc_16();
    const lv_font_t* font_right = font_noto_sc_16();

    lv_style_init(&s_content);
    lv_style_set_bg_color(&s_content, lv_color_hex(0x101010));
    lv_style_set_bg_grad_color(&s_content, lv_color_hex(0x101010));
    lv_style_set_bg_grad_dir(&s_content, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&s_content, LV_OPA_COVER);
    lv_style_set_border_width(&s_content, 0);
    lv_style_set_border_opa(&s_content, LV_OPA_0);
    lv_style_set_outline_width(&s_content, 0);
    lv_style_set_outline_opa(&s_content, LV_OPA_0);
    lv_style_set_pad_all(&s_content, 0);

    lv_style_init(&s_list);
    lv_style_set_bg_color(&s_list, lv_color_hex(0x101010));
    lv_style_set_bg_opa(&s_list, LV_OPA_COVER);
    lv_style_set_border_width(&s_list, 0);
    lv_style_set_border_opa(&s_list, LV_OPA_0);
    lv_style_set_outline_width(&s_list, 0);
    lv_style_set_outline_opa(&s_list, LV_OPA_0);
    lv_style_set_pad_left(&s_list, (w >= 360) ? 12 : 8);
    lv_style_set_pad_right(&s_list, (w >= 360) ? 10 : 6);
    lv_style_set_pad_top(&s_list, 4);
    lv_style_set_pad_bottom(&s_list, 4);

    lv_style_init(&s_row);
    lv_style_set_radius(&s_row, 0);
    lv_style_set_border_width(&s_row, 0);
    lv_style_set_shadow_width(&s_row, 0);
    lv_style_set_outline_width(&s_row, 0);
    lv_style_set_bg_color(&s_row, lv_color_hex(0x101010));
    lv_style_set_bg_grad_dir(&s_row, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&s_row, LV_OPA_COVER);
    lv_style_set_pad_all(&s_row, 0);

    lv_style_init(&s_row_checked);
    lv_style_set_bg_color(&s_row_checked, lv_color_hex(0x101010));
    lv_style_set_bg_grad_dir(&s_row_checked, LV_GRAD_DIR_NONE);
    lv_style_set_bg_opa(&s_row_checked, LV_OPA_COVER);
    lv_style_set_border_width(&s_row_checked, 0);
    lv_style_set_border_opa(&s_row_checked, LV_OPA_0);

    lv_style_init(&s_label_left);
    lv_style_set_text_font(&s_label_left, font_main);
    lv_style_set_text_color(&s_label_left, lv_color_hex(0xe6e6e6));

    lv_style_init(&s_label_right);
    lv_style_set_text_font(&s_label_right, font_right);
    lv_style_set_text_color(&s_label_right, lv_color_hex(0xcfcfcf));

    lv_style_init(&s_label_checked);
    lv_style_set_text_color(&s_label_checked, lv_color_hex(0xffffff));
}

void apply_content(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_content, LV_PART_MAIN);
}

void apply_list(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_list, LV_PART_MAIN);
}

void apply_list_row(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_row, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_row_checked, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_list_label_left(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_label_left, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_label_checked, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_list_label_right(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_label_right, LV_PART_MAIN);
    lv_obj_add_style(obj, &s_label_checked, LV_PART_MAIN | LV_STATE_CHECKED);
}

void apply_checked_state(lv_obj_t* obj)
{
    lv_obj_add_state(obj, LV_STATE_CHECKED);
}

void clear_checked_state(lv_obj_t* obj)
{
    lv_obj_clear_state(obj, LV_STATE_CHECKED);
}

} // namespace lofi::ui::screens::list_page::styles
