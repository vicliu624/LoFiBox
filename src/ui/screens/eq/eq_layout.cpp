#include "ui/screens/eq/eq_layout.h"

// Wireframe (eq)
// +----------------------------------------------+
// | < Back              TITLE               BAT  |  topbar
// +----------------------------------------------+
// | +------------------------------------------+ |
// | | dB labels | sliders |                    | |
// | +------------------------------------------+ |
// | | Preset: Flat                              | |
// +----------------------------------------------+

namespace lofi::ui::screens::eq::layout {
namespace {
constexpr lv_coord_t kBaseScreenWidth = 480;
constexpr lv_coord_t kBaseScreenHeight = 222;
constexpr lv_coord_t kBaseSidePad = 10;
constexpr lv_coord_t kBaseTopPad = 6;
constexpr lv_coord_t kBaseBottomPad = 8;
constexpr lv_coord_t kBaseSliderWidth = 36;
constexpr lv_coord_t kBaseSliderHeight = 110;
constexpr lv_coord_t kBaseLabelGap = 4;

lv_coord_t scale_w(lv_coord_t value) {
  lv_coord_t w = lv_display_get_horizontal_resolution(nullptr);
  if (w <= 0) {
    return value;
  }
  return static_cast<lv_coord_t>((value * w) / kBaseScreenWidth);
}

lv_coord_t scale_h(lv_coord_t value) {
  lv_coord_t h = lv_display_get_vertical_resolution(nullptr);
  if (h <= 0) {
    return value;
  }
  return static_cast<lv_coord_t>((value * h) / kBaseScreenHeight);
}
} // namespace

EqLayout create(lv_obj_t *content) {
  EqLayout refs{};

  if (content) {
    lv_obj_update_layout(content);
  }

  lv_coord_t content_w = content ? lv_obj_get_width(content) : 0;
  lv_coord_t content_h = content ? lv_obj_get_height(content) : 0;
  if (content_w <= 0) {
    content_w = lv_display_get_horizontal_resolution(nullptr);
  }
  if (content_h <= 0) {
    lv_coord_t full_h = lv_display_get_vertical_resolution(nullptr);
    content_h = full_h > 0 ? full_h : scale_h(kBaseScreenHeight);
  }

  refs.panel = lv_obj_create(content);
  lv_obj_set_size(refs.panel, content_w, content_h);
  lv_obj_set_pos(refs.panel, 0, 0);
  lv_obj_clear_flag(refs.panel, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_update_layout(refs.panel);
  lv_coord_t panel_w = lv_obj_get_width(refs.panel);
  lv_coord_t panel_h = lv_obj_get_height(refs.panel);
  if (panel_w <= 0) {
    panel_w = content_w;
  }
  if (panel_h <= 0) {
    panel_h = content_h;
  }
  lv_coord_t pad_l = scale_w(kBaseSidePad);
  lv_coord_t pad_r = scale_w(kBaseSidePad);
  lv_coord_t pad_t = scale_h(kBaseTopPad);
  lv_coord_t pad_b = scale_h(kBaseBottomPad);
  lv_coord_t preset_h = scale_h(24);
  lv_coord_t sep_h = 1;
  lv_coord_t preset_gap = scale_h(10);
  lv_coord_t graph_h = panel_h - pad_t - pad_b - preset_h - sep_h - preset_gap;
  if (graph_h < scale_h(80)) {
    graph_h = scale_h(80);
  }

  lv_coord_t graph_x = pad_l;
  lv_coord_t graph_y = pad_t;
  lv_coord_t graph_total_h = graph_h + preset_gap + preset_h;
  lv_coord_t extra_y = panel_h - pad_t - pad_b - graph_total_h;
  if (extra_y > 0) {
    graph_y += extra_y / 2;
  }
  lv_coord_t graph_w = panel_w - pad_l - pad_r;
  refs.graph = lv_obj_create(refs.panel);
  lv_obj_set_pos(refs.graph, graph_x, graph_y);
  lv_obj_set_size(refs.graph, graph_w, graph_h);
  lv_obj_clear_flag(refs.graph, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_move_background(refs.graph);

  lv_coord_t graph_inner_h = graph_h;
  lv_coord_t gap = (graph_w * 2) / 100;
  if (gap < scale_w(2)) {
    gap = scale_w(2);
  }
  lv_coord_t label_w = (graph_w * 20) / 100;
  lv_coord_t db_w = (graph_w * 9) / 100;
  lv_coord_t slider_area_w = (graph_w * 64) / 100;
  lv_coord_t right_pad = graph_w - label_w - db_w - gap * 2 - slider_area_w;
  lv_coord_t min_slider_area = scale_w(120);
  if (slider_area_w < min_slider_area) {
    slider_area_w = min_slider_area;
    lv_coord_t remaining = graph_w - slider_area_w - gap * 2;
    lv_coord_t min_db = scale_w(30);
    if (remaining < min_db) {
      remaining = min_db;
    }
    db_w = remaining / 5;
    if (db_w < min_db) {
      db_w = min_db;
    }
    label_w = remaining - db_w;
    if (label_w < scale_w(34)) {
      label_w = scale_w(34);
    }
    right_pad = graph_w - label_w - db_w - gap * 2 - slider_area_w;
  }
  if (right_pad < 0) {
    right_pad = 0;
  }

  lv_obj_t *label_col = lv_obj_create(refs.panel);
  lv_obj_set_pos(label_col, graph_x, graph_y);
  lv_obj_set_size(label_col, label_w, graph_inner_h);
  lv_obj_clear_flag(label_col, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(label_col, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(label_col, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(label_col, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(label_col, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(label_col, 0, LV_PART_MAIN);
  lv_obj_move_foreground(label_col);

  lv_coord_t label_y = scale_h(6);
  lv_coord_t label_step = (graph_inner_h - scale_h(12)) / 6;
  for (int i = 0; i < 6; ++i) {
    refs.labels[i] = lv_label_create(label_col);
    lv_obj_set_pos(refs.labels[i], 0, label_y + i * label_step);
    lv_obj_set_width(refs.labels[i], label_w);
  }

  lv_obj_t *scale_col = lv_obj_create(refs.panel);
  lv_obj_set_pos(scale_col, graph_x + label_w + gap, graph_y);
  lv_obj_set_size(scale_col, db_w, graph_inner_h);
  lv_obj_clear_flag(scale_col, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(scale_col, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(scale_col, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(scale_col, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(scale_col, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(scale_col, 0, LV_PART_MAIN);
  lv_obj_move_foreground(scale_col);

  refs.db_top = lv_label_create(scale_col);
  refs.db_mid = lv_label_create(scale_col);
  refs.db_bottom = lv_label_create(scale_col);
  lv_obj_align(refs.db_top, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_align(refs.db_mid, LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(refs.db_bottom, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_label_set_text(refs.db_top, "+12 dB");
  lv_label_set_text(refs.db_mid, "0 dB");
  lv_label_set_text(refs.db_bottom, "-12 dB");

  lv_obj_t *slider_area = lv_obj_create(refs.panel);
  lv_coord_t slider_x = graph_x + label_w + db_w + gap * 2 + right_pad / 2;
  lv_obj_set_pos(slider_area, slider_x, graph_y);
  lv_obj_set_size(slider_area, slider_area_w, graph_inner_h);
  lv_obj_clear_flag(slider_area, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(slider_area, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(slider_area, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(slider_area, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(slider_area, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(slider_area, 0, LV_PART_MAIN);
  lv_obj_move_foreground(slider_area);

  lv_coord_t bottom_label_h = scale_h(24);
  lv_coord_t grid_h = graph_inner_h - bottom_label_h - scale_h(4);
  if (grid_h < scale_h(60)) {
    grid_h = scale_h(60);
  }

  lv_obj_t *grid = lv_obj_create(slider_area);
  lv_obj_set_pos(grid, 0, 0);
  lv_obj_set_size(grid, slider_area_w, grid_h);
  lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(grid, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(grid, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
  lv_obj_move_background(grid);

  constexpr int kGridLines = 9;
  for (int i = 0; i < kGridLines; ++i) {
    lv_obj_t *line = lv_obj_create(grid);
    lv_coord_t thickness = (i == (kGridLines / 2)) ? 2 : 1;
    lv_obj_set_size(line, LV_PCT(100), thickness);
    lv_color_t color = (i == (kGridLines / 2)) ? lv_color_hex(0x3b76ff)
                                               : lv_color_hex(0x2a2c30);
    lv_obj_set_style_bg_color(line, color, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(line, 0, LV_PART_MAIN);
    lv_coord_t y =
        (grid_h > 1)
            ? static_cast<lv_coord_t>((grid_h - 1) * i / (kGridLines - 1))
            : 0;
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, y);
  }

  lv_coord_t col_w = (slider_area_w > 0) ? (slider_area_w / 6) : scale_w(16);
  lv_coord_t slider_w = scale_w(kBaseSliderWidth);
  lv_coord_t max_slider_w = col_w - scale_w(6);
  if (max_slider_w < scale_w(8)) {
    max_slider_w = scale_w(8);
  }
  if (slider_w > max_slider_w) {
    slider_w = max_slider_w;
  }
  if (slider_w < scale_w(10)) {
    slider_w = scale_w(10);
  }
  lv_coord_t slider_h = scale_h(kBaseSliderHeight);
  lv_coord_t slider_margin = scale_h(6);
  if (slider_h > grid_h - slider_margin) {
    slider_h = grid_h - slider_margin;
  }
  if (slider_h < scale_h(48)) {
    slider_h = scale_h(48);
  }
  lv_coord_t label_gap = 0;

  lv_obj_set_height(slider_area, grid_h);

  for (int i = 0; i < 6; ++i) {
    lv_coord_t col_x = i * col_w;
    refs.slider_cols[i] = lv_obj_create(slider_area);
    lv_obj_set_pos(refs.slider_cols[i], col_x, 0);
    lv_obj_set_size(refs.slider_cols[i], col_w, grid_h);
    lv_obj_clear_flag(refs.slider_cols[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(refs.slider_cols[i], lv_color_hex(0x000000),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(refs.slider_cols[i], LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_border_width(refs.slider_cols[i], 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(refs.slider_cols[i], 0, LV_PART_MAIN);

    lv_obj_t *slider = lv_slider_create(refs.slider_cols[i]);
    lv_obj_set_size(slider, slider_w, slider_h);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
    lv_slider_set_range(slider, -12, 12);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    refs.sliders[i] = slider;

    refs.value_labels[i] = lv_label_create(refs.slider_cols[i]);
    lv_obj_set_width(refs.value_labels[i], col_w);
    lv_obj_align(refs.value_labels[i], LV_ALIGN_CENTER, 0, 0);
  }

  lv_obj_t *bottom_labels = lv_obj_create(refs.panel);
  lv_obj_set_pos(bottom_labels, slider_x, graph_y + grid_h + label_gap);
  lv_obj_set_size(bottom_labels, slider_area_w, bottom_label_h);
  lv_obj_clear_flag(bottom_labels, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(bottom_labels, lv_color_hex(0x000000),
                            LV_PART_MAIN);
  lv_obj_set_style_bg_opa(bottom_labels, LV_OPA_0, LV_PART_MAIN);
  lv_obj_set_style_border_width(bottom_labels, 0, LV_PART_MAIN);
  lv_obj_set_style_outline_width(bottom_labels, 0, LV_PART_MAIN);
  lv_obj_set_style_pad_all(bottom_labels, 0, LV_PART_MAIN);
  lv_obj_move_foreground(bottom_labels);

  for (int i = 0; i < 6; ++i) {
    lv_coord_t col_x = i * col_w;
    refs.labels[i + 6] = lv_label_create(bottom_labels);
    lv_obj_set_pos(refs.labels[i + 6], col_x, scale_h(2));
    lv_obj_set_width(refs.labels[i + 6], col_w);
    lv_obj_set_height(refs.labels[i + 6], bottom_label_h);
  }

  for (int i = 0; i < 6; ++i) {
    lv_obj_t *vline = lv_obj_create(grid);
    lv_obj_set_size(vline, 1, grid_h);
    lv_obj_set_style_bg_color(vline, lv_color_hex(0x23262b), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(vline, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(vline, 0, LV_PART_MAIN);
    lv_coord_t x = i * col_w + col_w / 2;
    lv_obj_align(vline, LV_ALIGN_TOP_LEFT, x, 0);
  }

  lv_obj_t *preset_sep = lv_obj_create(refs.panel);
  lv_obj_set_pos(preset_sep, pad_l, pad_t + graph_h + preset_gap - 2);
  lv_obj_set_size(preset_sep, panel_w - pad_l - pad_r, 1);
  lv_obj_set_style_bg_color(preset_sep, lv_color_hex(0x2d3136), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(preset_sep, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(preset_sep, 0, LV_PART_MAIN);

  lv_obj_t *preset_row = lv_obj_create(refs.panel);
  lv_obj_set_pos(preset_row, pad_l, pad_t + graph_h + preset_gap);
  lv_obj_set_size(preset_row, panel_w - pad_l - pad_r, preset_h);
  lv_obj_clear_flag(preset_row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(preset_row, lv_color_hex(0x0b0c0e), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(preset_row, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_width(preset_row, 0, LV_PART_MAIN);
  lv_obj_set_style_radius(preset_row, 6, LV_PART_MAIN);
  lv_obj_set_style_pad_left(preset_row, scale_w(8), LV_PART_MAIN);
  lv_obj_set_style_pad_right(preset_row, scale_w(8), LV_PART_MAIN);
  lv_obj_set_style_pad_top(preset_row, scale_h(3), LV_PART_MAIN);
  lv_obj_set_style_pad_bottom(preset_row, scale_h(3), LV_PART_MAIN);

  refs.preset = lv_label_create(preset_row);
  lv_label_set_text(refs.preset, "Preset:");
  refs.preset_value = lv_label_create(preset_row);
  lv_label_set_text(refs.preset_value, "Flat");
  lv_obj_align(refs.preset, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_align(refs.preset_value, LV_ALIGN_LEFT_MID, scale_w(60), 0);

  refs.key_sink = lv_obj_create(refs.panel);
  lv_obj_set_size(refs.key_sink, 1, 1);
  lv_obj_add_flag(refs.key_sink, LV_OBJ_FLAG_HIDDEN);

  return refs;
}

} // namespace lofi::ui::screens::eq::layout
