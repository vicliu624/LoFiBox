#include "ui/screens/common/shell_styles.h"
#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::common::styles
{
namespace
{
static bool s_inited = false;
static lv_style_t s_root;
static lv_style_t s_topbar;
static lv_style_t s_topbar_label;
static lv_style_t s_topbar_title;
static lv_style_t s_topbar_status;
static lv_style_t s_topbar_fade_left;
static lv_style_t s_topbar_fade_right;
}

void init_once()
{
    if (s_inited) {
        return;
    }
    s_inited = true;

    lv_style_init(&s_root);
    lv_style_set_bg_color(&s_root, lv_color_hex(0x0b0b0b));
    lv_style_set_bg_opa(&s_root, LV_OPA_COVER);
    lv_style_set_border_width(&s_root, 0);
    lv_style_set_pad_all(&s_root, 0);
    lv_style_set_radius(&s_root, 0);

    lv_style_init(&s_topbar);
    lv_style_set_bg_color(&s_topbar, lv_color_hex(0x2b2b2b));
    lv_style_set_bg_grad_color(&s_topbar, lv_color_hex(0x0f0f0f));
    lv_style_set_bg_grad_dir(&s_topbar, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&s_topbar, LV_OPA_COVER);
    lv_style_set_border_width(&s_topbar, 1);
    lv_style_set_border_side(&s_topbar, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&s_topbar, lv_color_hex(0x3a3a3a));
    lv_style_set_pad_hor(&s_topbar, 10);
    lv_style_set_pad_ver(&s_topbar, 4);
    lv_style_set_radius(&s_topbar, 0);

    lv_style_init(&s_topbar_label);
    lv_style_set_text_font(&s_topbar_label, font_noto_sc_16());
    lv_style_set_text_color(&s_topbar_label, lv_color_hex(0xe6e6e6));

    lv_style_init(&s_topbar_title);
    lv_style_set_text_font(&s_topbar_title, font_noto_sc_16());
    lv_style_set_text_color(&s_topbar_title, lv_color_hex(0xf2f2f2));

    lv_style_init(&s_topbar_status);
    lv_style_set_bg_opa(&s_topbar_status, LV_OPA_0);
    lv_style_set_border_width(&s_topbar_status, 0);
    lv_style_set_pad_column(&s_topbar_status, 6);

    lv_style_init(&s_topbar_fade_left);
    lv_style_set_bg_color(&s_topbar_fade_left, lv_color_hex(0x2b2b2b));
    lv_style_set_bg_grad_color(&s_topbar_fade_left, lv_color_hex(0x2b2b2b));
    lv_style_set_bg_grad_dir(&s_topbar_fade_left, LV_GRAD_DIR_HOR);
    lv_style_set_bg_opa(&s_topbar_fade_left, LV_OPA_COVER);
    lv_style_set_bg_grad_opa(&s_topbar_fade_left, LV_OPA_TRANSP);
    lv_style_set_border_width(&s_topbar_fade_left, 0);

    lv_style_init(&s_topbar_fade_right);
    lv_style_set_bg_color(&s_topbar_fade_right, lv_color_hex(0x2b2b2b));
    lv_style_set_bg_grad_color(&s_topbar_fade_right, lv_color_hex(0x2b2b2b));
    lv_style_set_bg_grad_dir(&s_topbar_fade_right, LV_GRAD_DIR_HOR);
    lv_style_set_bg_opa(&s_topbar_fade_right, LV_OPA_TRANSP);
    lv_style_set_bg_grad_opa(&s_topbar_fade_right, LV_OPA_COVER);
    lv_style_set_border_width(&s_topbar_fade_right, 0);
}

void apply_root(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_root, LV_PART_MAIN);
}

void apply_topbar(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar, LV_PART_MAIN);
}

void apply_topbar_label(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar_label, LV_PART_MAIN);
}

void apply_topbar_title(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar_title, LV_PART_MAIN);
}

void apply_topbar_status(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar_status, LV_PART_MAIN);
}

void apply_topbar_fade_left(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar_fade_left, LV_PART_MAIN);
}

void apply_topbar_fade_right(lv_obj_t* obj)
{
    lv_obj_add_style(obj, &s_topbar_fade_right, LV_PART_MAIN);
}

} // namespace lofi::ui::screens::common::styles
