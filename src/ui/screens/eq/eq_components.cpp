#include "ui/screens/eq/eq_components.h"

#include <stdio.h>

#include "app/eq_dsp.h"
#include "ui/screens/eq/eq_input.h"
#include "ui/screens/eq/eq_layout.h"
#include "ui/screens/eq/eq_styles.h"

namespace lofi::ui::screens::eq {
namespace {
constexpr int kVisibleBands = 6;
const int kBandMap[kVisibleBands] = {0, 1, 2, 3, 4, 5};
const char *kLeftLabels[kVisibleBands] = {"120 Hz", "250 Hz",  "500 Hz",
                                          "1 kHz",  "2.5 kHz", "6 kHz"};
const char *kBottomLabels[kVisibleBands] = {"120", "250",  "500",
                                            "1k",  "2.5k", "6k"};

void format_db_label(char *out, size_t len, int8_t db) {
  if (!out || len == 0) {
    return;
  }
  if (db > 0) {
    snprintf(out, len, "+%d dB", db);
    return;
  }
  snprintf(out, len, "%d dB", db);
}

void apply_selection(UiScreen &screen) {
  int sel = screen.state.eq_selected_band;
  if (sel < 0) {
    sel = 0;
  }
  if (sel >= kVisibleBands) {
    sel = kVisibleBands - 1;
  }
  screen.state.eq_selected_band = sel;

  for (int i = 0; i < kVisibleBands; ++i) {
    if (!screen.view.eq.sliders[i]) {
      continue;
    }
    if (i == sel) {
      lv_obj_add_state(screen.view.eq.sliders[i], LV_STATE_CHECKED);
      if (screen.state.eq_editing) {
        lv_obj_add_state(screen.view.eq.sliders[i], LV_STATE_USER_1);
      } else {
        lv_obj_clear_state(screen.view.eq.sliders[i], LV_STATE_USER_1);
      }
    } else {
      lv_obj_clear_state(screen.view.eq.sliders[i], LV_STATE_CHECKED);
      lv_obj_clear_state(screen.view.eq.sliders[i], LV_STATE_USER_1);
    }
    if (screen.view.eq.value_labels[i]) {
      if (i == sel) {
        lv_obj_add_state(screen.view.eq.value_labels[i], LV_STATE_CHECKED);
        if (screen.state.eq_editing) {
          lv_obj_add_state(screen.view.eq.value_labels[i], LV_STATE_USER_1);
        } else {
          lv_obj_clear_state(screen.view.eq.value_labels[i], LV_STATE_USER_1);
        }
      } else {
        lv_obj_clear_state(screen.view.eq.value_labels[i], LV_STATE_CHECKED);
        lv_obj_clear_state(screen.view.eq.value_labels[i], LV_STATE_USER_1);
      }
    }
  }
}
} // namespace

void build(UiScreen &screen) {
  styles::init_once();
  screen.view.list = {};
  screen.row_count = 0;
  screen.view.eq = {};
  screen.state.eq_editing = false;
  screen.view.eq = layout::create(screen.view.root.content);

  if (screen.view.root.content) {
    styles::apply_content(screen.view.root.content);
  }
  styles::apply_panel(screen.view.eq.panel);
  styles::apply_graph(screen.view.eq.graph);
  styles::apply_db_label(screen.view.eq.db_top);
  styles::apply_db_label(screen.view.eq.db_mid);
  styles::apply_db_label(screen.view.eq.db_bottom);
  styles::apply_preset(screen.view.eq.preset);
  styles::apply_preset_value(screen.view.eq.preset_value);

  for (int i = 0; i < kVisibleBands; ++i) {
    int band = kBandMap[i];
    if (screen.view.eq.sliders[i]) {
      styles::apply_slider(screen.view.eq.sliders[i]);
      styles::apply_slider_selected(screen.view.eq.sliders[i]);
      lv_slider_set_range(screen.view.eq.sliders[i], -12, 12);
      lv_slider_set_mode(screen.view.eq.sliders[i], LV_SLIDER_MODE_SYMMETRICAL);
      lv_slider_set_value(screen.view.eq.sliders[i], app::eq::get_band(band),
                          LV_ANIM_OFF);
    }
    if (screen.view.eq.value_labels[i]) {
      styles::apply_value_label(screen.view.eq.value_labels[i]);
      char db_text[12] = {0};
      format_db_label(db_text, sizeof(db_text), app::eq::get_band(band));
      lv_label_set_text(screen.view.eq.value_labels[i], db_text);
    }
    if (screen.view.eq.labels[i]) {
      styles::apply_label(screen.view.eq.labels[i]);
      lv_label_set_text(screen.view.eq.labels[i], kLeftLabels[i]);
      if (i == 0) {
        lv_obj_set_style_text_color(screen.view.eq.labels[i],
                                    lv_color_hex(0xffffff), LV_PART_MAIN);
      } else {
        lv_obj_set_style_text_color(screen.view.eq.labels[i],
                                    lv_color_hex(0xd8dadd), LV_PART_MAIN);
      }
      lv_obj_set_style_text_align(screen.view.eq.labels[i], LV_TEXT_ALIGN_LEFT,
                                  LV_PART_MAIN);
      lv_obj_set_style_pad_bottom(screen.view.eq.labels[i], 4, LV_PART_MAIN);
    }
    if (screen.view.eq.labels[i + 6]) {
      styles::apply_label(screen.view.eq.labels[i + 6]);
      lv_label_set_text(screen.view.eq.labels[i + 6], kBottomLabels[i]);
      lv_obj_set_style_text_color(screen.view.eq.labels[i + 6],
                                  lv_color_hex(0x9ba1a8), LV_PART_MAIN);
      lv_obj_set_style_text_align(screen.view.eq.labels[i + 6],
                                  LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
  }

  apply_selection(screen);
  input::attach(screen, screen.view.eq.key_sink);
}

void update(UiScreen &screen) {
  if (screen.state.current != PageId::Eq) {
    return;
  }
  for (int i = 0; i < kVisibleBands; ++i) {
    int band = kBandMap[i];
    if (screen.view.eq.sliders[i]) {
      lv_slider_set_value(screen.view.eq.sliders[i], app::eq::get_band(band),
                          LV_ANIM_OFF);
    }
    if (screen.view.eq.value_labels[i]) {
      char db_text[12] = {0};
      format_db_label(db_text, sizeof(db_text), app::eq::get_band(band));
      lv_label_set_text(screen.view.eq.value_labels[i], db_text);
    }
  }
  apply_selection(screen);
}

} // namespace lofi::ui::screens::eq
