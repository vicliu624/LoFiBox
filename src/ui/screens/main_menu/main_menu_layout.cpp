#include "ui/screens/main_menu/main_menu_layout.h"
#include "ui/screens/common/shell_layout.h"
#include "ui/fonts/fonts.h"

namespace lofi::ui::screens::main_menu::layout
{
namespace
{
constexpr lv_coord_t kBaseScreenWidth = 480;
constexpr lv_coord_t kBaseScreenHeight = 222;
constexpr lv_coord_t kBaseIconSize = 134;
constexpr lv_coord_t kBaseIconTop = 60;
constexpr lv_coord_t kBaseIconGap = 34;
constexpr lv_coord_t kBaseLabelTop = 200;
constexpr lv_coord_t kBaseLabelHeight = 20;
constexpr lv_coord_t kBaseDotSize = 5;
constexpr lv_coord_t kBaseDotGap = 12;
constexpr lv_coord_t kBaseDotBottomMargin = 10;
constexpr lv_coord_t kBaseDotLabelGap = 12;
constexpr lv_coord_t kBaseLabelIconGap = 10;
constexpr lv_coord_t kBaseLabelYOffset = -6;
constexpr lv_coord_t kBaseMenuOffsetY = -10;
constexpr lv_coord_t kBaseArrowInset = 6;
constexpr lv_coord_t kBaseArrowOffsetY = -12;
constexpr lv_coord_t kBaseRowOffset = -3;
constexpr lv_coord_t kBaseRowOffsetY = -18;

lv_coord_t scale_x(lv_coord_t value, lv_coord_t width)
{
    if (width <= 0) {
        return value;
    }
    return static_cast<lv_coord_t>((value * width) / kBaseScreenWidth);
}

lv_coord_t scale_y(lv_coord_t value, lv_coord_t height)
{
    if (height <= 0) {
        return value;
    }
    return static_cast<lv_coord_t>((value * height) / kBaseScreenHeight);
}

lv_coord_t scale_min(lv_coord_t value, lv_coord_t width, lv_coord_t height)
{
    lv_coord_t sx = scale_x(value, width);
    lv_coord_t sy = scale_y(value, height);
    return (sx < sy) ? sx : sy;
}
}

MenuLayout create_menu(lv_obj_t* content)
{
    MenuLayout refs{};
    if (!content) {
        return refs;
    }

    refs.wrap = lv_obj_create(content);
    lv_obj_set_size(refs.wrap, LV_PCT(100), LV_PCT(100));
    lv_obj_align(refs.wrap, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_clear_flag(refs.wrap, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(refs.wrap, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_bg_opa(refs.wrap, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(refs.wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_border_opa(refs.wrap, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(refs.wrap, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_opa(refs.wrap, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(refs.wrap, 0, LV_PART_MAIN);

    lv_obj_update_layout(content);
    lv_coord_t width = lv_obj_get_width(content);
    lv_coord_t height = lv_obj_get_height(content);
    if (width <= 0) {
        width = lv_display_get_horizontal_resolution(nullptr);
    }
    if (height <= 0) {
        lv_coord_t full_h = lv_display_get_vertical_resolution(nullptr);
        lv_coord_t top_h = lofi::ui::screens::common::layout::topbar_height();
        height = (full_h > top_h) ? (full_h - top_h) : full_h;
    }

    lv_coord_t icon_size = scale_min(kBaseIconSize, width, height);
    lv_coord_t icon_gap = scale_x(kBaseIconGap, width);
    const lv_font_t* label_font = font_noto_sc_16();
    lv_coord_t label_height = lv_font_get_line_height(label_font);

    lv_coord_t dot_size = scale_min(kBaseDotSize, width, height);
    if (dot_size < 2) {
        dot_size = 2;
    }
    lv_coord_t dot_bottom = height - scale_y(kBaseDotBottomMargin, height);
    lv_coord_t dot_top = dot_bottom - dot_size;
    lv_coord_t label_gap = scale_y(kBaseDotLabelGap, height);
    lv_coord_t icon_gap_y = scale_y(kBaseLabelIconGap, height);
    lv_coord_t menu_shift = scale_y(kBaseMenuOffsetY, height);
    dot_top += menu_shift;
    lv_coord_t max_icon = dot_top - label_gap - label_height - icon_gap_y;
    if (max_icon < 1) {
        max_icon = 1;
    }
    if (icon_size > max_icon) {
        icon_size = max_icon;
    }
    lv_coord_t label_top = dot_top - label_gap - label_height + scale_y(kBaseLabelYOffset, height);
    lv_coord_t icon_top = label_top - icon_gap_y - icon_size;
    if (icon_top < 0) {
        lv_coord_t delta = -icon_top;
        icon_top += delta;
        label_top += delta;
        dot_top += delta;
    }
    if (dot_top + dot_size > height) {
        lv_coord_t delta = (dot_top + dot_size) - height;
        icon_top -= delta;
        label_top -= delta;
        dot_top -= delta;
    }

    lv_coord_t row_width = icon_size * kVisibleItems + icon_gap * (kVisibleItems - 1);
    lv_coord_t row_left = (width - row_width) / 2 + scale_x(kBaseRowOffset, width);
    lv_coord_t item_height = label_top + label_height - icon_top;

    refs.icon_size = icon_size;
    refs.icon_top = icon_top;
    refs.icon_gap = icon_gap;
    refs.label_top = label_top;
    refs.label_height = label_height;
    refs.row_left = row_left;
    refs.item_height = item_height;

    for (int i = 0; i < kVisibleItems; ++i) {
        MenuItemLayout& item = refs.items[i];
        item.button = lv_btn_create(refs.wrap);
        lv_obj_remove_style_all(item.button);
        lv_obj_set_size(item.button, icon_size, item_height);
        lv_obj_align(item.button, LV_ALIGN_TOP_LEFT, row_left + i * (icon_size + icon_gap), icon_top);
        lv_obj_clear_flag(item.button, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(item.button, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
        lv_obj_set_style_pad_all(item.button, 0, LV_PART_MAIN);

        item.icon = lv_img_create(item.button);
        lv_obj_set_size(item.icon, icon_size, icon_size);
        lv_obj_align(item.icon, LV_ALIGN_TOP_MID, 0, 0);

        item.label = lv_label_create(item.button);
        lv_label_set_long_mode(item.label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(item.label, icon_size + icon_gap);
        lv_obj_align(item.label, LV_ALIGN_TOP_MID, 0, label_top - icon_top);
        lv_obj_set_style_text_align(item.label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }

    refs.arrow_left = lv_label_create(refs.wrap);
    lv_label_set_text(refs.arrow_left, LV_SYMBOL_LEFT);
    lv_obj_update_layout(refs.arrow_left);
    lv_coord_t arrow_h = lv_obj_get_height(refs.arrow_left);
    lv_coord_t arrow_y = icon_top + (icon_size / 2) - (arrow_h / 2) + kBaseArrowOffsetY;
    lv_obj_set_pos(refs.arrow_left, scale_x(kBaseArrowInset, width), arrow_y);

    refs.arrow_right = lv_label_create(refs.wrap);
    lv_label_set_text(refs.arrow_right, LV_SYMBOL_RIGHT);
    lv_obj_update_layout(refs.arrow_right);
    lv_coord_t arrow_w = lv_obj_get_width(refs.arrow_right);
    lv_obj_set_pos(refs.arrow_right, width - scale_x(kBaseArrowInset, width) - arrow_w, arrow_y);

    lv_coord_t dot_gap = scale_x(kBaseDotGap, width);
    lv_coord_t dots_width = dot_size * kDots + dot_gap * (kDots - 1);
    lv_coord_t dots_left = (width - dots_width) / 2;
    lv_coord_t dots_top = dot_top;

    for (int i = 0; i < kDots; ++i) {
        refs.dots[i] = lv_obj_create(refs.wrap);
        lv_obj_set_size(refs.dots[i], dot_size, dot_size);
        lv_obj_align(refs.dots[i], LV_ALIGN_TOP_LEFT, dots_left + i * (dot_size + dot_gap), dots_top);
        lv_obj_clear_flag(refs.dots[i], LV_OBJ_FLAG_SCROLLABLE);
    }

    return refs;
}

void set_item(MenuItemLayout& item, const char* label, const lv_image_dsc_t* icon)
{
    if (item.label) {
        lv_label_set_text(item.label, label ? label : "");
    }
    if (item.icon) {
        if (icon) {
            lv_img_set_src(item.icon, icon);
            if (icon->header.w > 0) {
                lv_coord_t target = lv_obj_get_width(item.icon);
                if (target < 1) {
                    target = icon->header.w;
                }
                uint32_t zoom = (static_cast<uint32_t>(target) * 256U) / static_cast<uint32_t>(icon->header.w);
                if (zoom == 0) {
                    zoom = 256;
                }
                lv_img_set_zoom(item.icon, zoom);
            }
            lv_obj_clear_flag(item.icon, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(item.icon, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void set_dot_active(lv_obj_t* dot, bool active)
{
    if (!dot) {
        return;
    }
    if (active) {
        lv_obj_add_state(dot, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(dot, LV_STATE_CHECKED);
    }
}

} // namespace lofi::ui::screens::main_menu::layout
