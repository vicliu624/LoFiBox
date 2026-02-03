#include "LV_Helper.h"

#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <stdlib.h>
#include <string.h>

#include "app/input_keys.h"
#include "board/BoardBase.h"
#include "ui/lofibox/lofibox_ui.h"

extern bool ui_take_screenshot_to_sd();
extern bool ui_process_screenshot();

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 240
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 135
#endif

namespace {
static lv_display_t *s_display = nullptr;
static lv_color_t *s_buf1 = nullptr;
static lv_color_t *s_buf2 = nullptr;
static lv_color_t *s_framebuffer = nullptr;
static uint16_t s_fb_width = 0;
static uint16_t s_fb_height = 0;
static bool s_fb_ready = false;
static lv_indev_t *s_indev_keyboard = nullptr;
static bool s_key_pending = false;
static uint32_t s_last_key = 0;
static lv_fs_drv_t s_sd_drv;
static uint32_t s_last_input_ms = 0;
static bool s_display_sleep = false;
static uint8_t s_saved_brightness = 0;
static uint8_t s_saved_kb_brightness = 0;
static uint32_t s_backlight_timeout_ms = 10000;
static uint32_t s_sleep_timeout_ms = 0;
static bool s_settings_loaded = false;
static bool s_sleep_requested = false;

constexpr uint32_t kMinTimeoutMs = 1000;
constexpr char kPrefsNamespace[] = "ui";
constexpr char kPrefsKeyBacklight[] = "bl_ms";

static void wake_display() {
  if (!s_display_sleep) {
    return;
  }
  s_display_sleep = false;
  if (s_saved_brightness == 0) {
    s_saved_brightness = 1;
  }
  board.setBrightness(s_saved_brightness);
  if (board.hasKeyboard()) {
    board.keyboardSetBrightness(s_saved_kb_brightness);
  }
}

static void note_input_activity() {
  s_last_input_ms = millis();
  s_sleep_requested = false;
  wake_display();
}

static uint32_t clamp_timeout(uint32_t value) {
  if (value != 0 && value < kMinTimeoutMs) {
    return kMinTimeoutMs;
  }
  return value;
}

static void load_settings() {
  if (s_settings_loaded) {
    return;
  }
  s_settings_loaded = true;
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  s_backlight_timeout_ms =
      clamp_timeout(prefs.getUInt(kPrefsKeyBacklight, s_backlight_timeout_ms));
  prefs.end();
}

static void save_settings() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false)) {
    return;
  }
  prefs.putUInt(kPrefsKeyBacklight, s_backlight_timeout_ms);
  prefs.end();
}

uint8_t brightness_level_for_index(int idx) {
  if (idx < 0) {
    idx = 0;
  }
  if (idx > 4) {
    idx = 4;
  }
  const uint8_t max_level = DEVICE_MAX_BRIGHTNESS_LEVEL;
  if (max_level <= 1) {
    return max_level;
  }
  const uint8_t level = static_cast<uint8_t>((max_level * (idx + 1)) / 5);
  return level == 0 ? 1 : level;
}

void cycle_brightness() {
  uint8_t current =
      s_display_sleep ? s_saved_brightness : board.getBrightness();
  uint8_t levels[5];
  for (int i = 0; i < 5; ++i) {
    levels[i] = brightness_level_for_index(i);
  }

  int next_idx = 0;
  bool found = false;
  for (int i = 0; i < 5; ++i) {
    if (current <= levels[i]) {
      next_idx = (i + 1) % 5;
      found = true;
      break;
    }
  }
  if (!found) {
    next_idx = 0;
  }

  uint8_t next = levels[next_idx];
  s_saved_brightness = next;
  s_display_sleep = false;
  board.setBrightness(next);
}

void toggle_screen_power() {
  if (!s_display_sleep) {
    s_saved_brightness = board.getBrightness();
    if (board.hasKeyboard()) {
      s_saved_kb_brightness = board.keyboardGetBrightness();
    }
    board.setBrightness(0);
    if (board.hasKeyboard()) {
      board.keyboardSetBrightness(0);
    }
    s_display_sleep = true;
    return;
  }

  s_display_sleep = false;
  if (s_saved_brightness == 0) {
    s_saved_brightness = 1;
  }
  board.setBrightness(s_saved_brightness);
  if (board.hasKeyboard()) {
    board.keyboardSetBrightness(s_saved_kb_brightness);
  }
}

static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area,
                       uint8_t *color_p) {
  (void)disp_drv;
  int32_t w = area->x2 - area->x1 + 1;
  int32_t h = area->y2 - area->y1 + 1;

  if (s_framebuffer && w > 0 && h > 0) {
    const uint8_t *src = color_p;
    const int32_t row_bytes = w * static_cast<int32_t>(sizeof(lv_color_t));
    for (int32_t y = 0; y < h; ++y) {
      uint16_t *dst = reinterpret_cast<uint16_t *>(s_framebuffer) +
                      (area->y1 + y) * s_fb_width + area->x1;
      memcpy(dst, src, row_bytes);
      src += row_bytes;
    }
    s_fb_ready = true;
  }

#if defined(BOARD_TLORA_PAGER)
  lv_draw_sw_rgb565_swap(color_p, w * h);
#endif

  board.displayPushColors(area->x1, area->y1, w, h,
                          reinterpret_cast<uint16_t *>(color_p));
  lv_display_flush_ready(disp_drv);
}

static uint32_t lv_tick_get_callback() { return millis(); }

static void keypad_read(lv_indev_t *drv, lv_indev_data_t *data) {
  (void)drv;
  if (s_key_pending) {
    data->key = s_last_key;
    data->state = LV_INDEV_STATE_RELEASED;
    s_key_pending = false;
    return;
  }

  uint32_t key = 0;
  if (board.readKey(&key)) {
    bool was_sleep = s_display_sleep;
    note_input_activity();
    if (was_sleep) {
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_PLAY_PAUSE) {
      lofi::ui::handle_media_key(lofi::ui::MediaKey::PlayPause);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_NEXT) {
      lofi::ui::handle_media_key(lofi::ui::MediaKey::Next);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_PREV) {
      lofi::ui::handle_media_key(lofi::ui::MediaKey::Prev);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_VOL_CYCLE) {
      lofi::ui::handle_global_key(lofi::ui::GlobalKey::VolumeCycle);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_MODE_CYCLE) {
      lofi::ui::handle_global_key(lofi::ui::GlobalKey::PlaybackModeCycle);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_BRIGHTNESS_CYCLE) {
      cycle_brightness();
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_SCREEN_TOGGLE) {
      toggle_screen_power();
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_DELETE_PROMPT) {
      lofi::ui::handle_global_key(lofi::ui::GlobalKey::DeletePrompt);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_DELETE_CONFIRM) {
      lofi::ui::handle_global_key(lofi::ui::GlobalKey::DeleteConfirm);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_DELETE_CANCEL) {
      lofi::ui::handle_global_key(lofi::ui::GlobalKey::DeleteCancel);
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    if (key == APP_KEY_SCREENSHOT) {
      ui_take_screenshot_to_sd();
      data->state = LV_INDEV_STATE_RELEASED;
      return;
    }
    data->key = key;
    data->state = LV_INDEV_STATE_PRESSED;
    s_last_key = key;
    s_key_pending = true;
    return;
  }

  data->state = LV_INDEV_STATE_RELEASED;
}

static bool sd_ready_cb(lv_fs_drv_t *drv) {
  (void)drv;
  return board.isSDReady();
}

static void *sd_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) {
  (void)drv;
  const char *fs_path = path && path[0] ? path : "/";
  const char *open_mode = (mode & LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ;
  File file = SD.open(fs_path, open_mode);
  if (!file) {
    return nullptr;
  }
  File *handle = new File(file);
  return handle;
}

static lv_fs_res_t sd_close_cb(lv_fs_drv_t *drv, void *file_p) {
  (void)drv;
  File *handle = static_cast<File *>(file_p);
  if (!handle) {
    return LV_FS_RES_INV_PARAM;
  }
  handle->close();
  delete handle;
  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf,
                              uint32_t btr, uint32_t *br) {
  (void)drv;
  File *handle = static_cast<File *>(file_p);
  if (!handle) {
    return LV_FS_RES_INV_PARAM;
  }
  size_t read_bytes = handle->read(static_cast<uint8_t *>(buf), btr);
  if (br) {
    *br = static_cast<uint32_t>(read_bytes);
  }
  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_write_cb(lv_fs_drv_t *drv, void *file_p, const void *buf,
                               uint32_t btw, uint32_t *bw) {
  (void)drv;
  File *handle = static_cast<File *>(file_p);
  if (!handle) {
    return LV_FS_RES_INV_PARAM;
  }
  size_t written = handle->write(static_cast<const uint8_t *>(buf), btw);
  if (bw) {
    *bw = static_cast<uint32_t>(written);
  }
  return LV_FS_RES_OK;
}

static lv_fs_res_t sd_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                              lv_fs_whence_t whence) {
  (void)drv;
  File *handle = static_cast<File *>(file_p);
  if (!handle) {
    return LV_FS_RES_INV_PARAM;
  }
  bool ok = false;
  if (whence == LV_FS_SEEK_SET) {
    ok = handle->seek(pos, SeekSet);
  } else if (whence == LV_FS_SEEK_CUR) {
    ok = handle->seek(handle->position() + pos, SeekSet);
  } else if (whence == LV_FS_SEEK_END) {
    ok = handle->seek(handle->size() + pos, SeekSet);
  }
  return ok ? LV_FS_RES_OK : LV_FS_RES_FS_ERR;
}

static lv_fs_res_t sd_tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) {
  (void)drv;
  File *handle = static_cast<File *>(file_p);
  if (!handle || !pos_p) {
    return LV_FS_RES_INV_PARAM;
  }
  *pos_p = static_cast<uint32_t>(handle->position());
  return LV_FS_RES_OK;
}
} // namespace

void beginLvglHelper() {
  load_settings();
  lv_init();

  lv_fs_drv_init(&s_sd_drv);
  s_sd_drv.letter = 'S';
  s_sd_drv.ready_cb = sd_ready_cb;
  s_sd_drv.open_cb = sd_open_cb;
  s_sd_drv.close_cb = sd_close_cb;
  s_sd_drv.read_cb = sd_read_cb;
  s_sd_drv.write_cb = sd_write_cb;
  s_sd_drv.seek_cb = sd_seek_cb;
  s_sd_drv.tell_cb = sd_tell_cb;
  lv_fs_drv_register(&s_sd_drv);

  uint16_t width = board.displayWidth();
  uint16_t height = board.displayHeight();
  if (width == 0 || height == 0) {
    width = SCREEN_WIDTH;
    height = SCREEN_HEIGHT;
  }

  const uint32_t buffer_pixels = (width * height) / 6;
  const size_t buffer_size = buffer_pixels * sizeof(lv_color_t);

  s_buf1 = static_cast<lv_color_t *>(malloc(buffer_size));
  s_buf2 = static_cast<lv_color_t *>(malloc(buffer_size));
  s_framebuffer = static_cast<lv_color_t *>(
      malloc(static_cast<size_t>(width) * height * sizeof(lv_color_t)));
  if (s_framebuffer) {
    memset(s_framebuffer, 0,
           static_cast<size_t>(width) * height * sizeof(lv_color_t));
    s_fb_width = static_cast<uint16_t>(width);
    s_fb_height = static_cast<uint16_t>(height);
    s_fb_ready = false;
  }

  s_display = lv_display_create(width, height);
  lv_display_set_buffers(s_display, s_buf1, s_buf2, buffer_size,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_color_format(s_display, LV_COLOR_FORMAT_RGB565);
  lv_display_set_flush_cb(s_display, disp_flush);
  lv_tick_set_cb(lv_tick_get_callback);

  s_indev_keyboard = lv_indev_create();
  lv_indev_set_type(s_indev_keyboard, LV_INDEV_TYPE_KEYPAD);
  lv_indev_set_read_cb(s_indev_keyboard, keypad_read);
  lv_indev_set_display(s_indev_keyboard, s_display);
  lv_indev_set_group(s_indev_keyboard, lv_group_get_default());

  s_last_input_ms = millis();
  s_saved_brightness = board.getBrightness();
  if (board.hasKeyboard()) {
    s_saved_kb_brightness = board.keyboardGetBrightness();
  }
}

const uint16_t *lvHelperGetFrameBuffer(uint16_t *width, uint16_t *height) {
  if (!s_framebuffer || !s_fb_ready) {
    return nullptr;
  }
  if (width) {
    *width = s_fb_width;
  }
  if (height) {
    *height = s_fb_height;
  }
  return reinterpret_cast<const uint16_t *>(s_framebuffer);
}

void lvHelperTick() {
  uint32_t now = millis();
  uint32_t idle_ms = static_cast<uint32_t>(now - s_last_input_ms);
  if (!s_display_sleep && s_backlight_timeout_ms > 0 &&
      idle_ms >= s_backlight_timeout_ms) {
    s_saved_brightness = board.getBrightness();
    if (board.hasKeyboard()) {
      s_saved_kb_brightness = board.keyboardGetBrightness();
    }
    board.setBrightness(0);
    if (board.hasKeyboard()) {
      board.keyboardSetBrightness(0);
    }
    s_display_sleep = true;
  }
  if (!s_sleep_requested && s_sleep_timeout_ms > 0 &&
      idle_ms >= s_sleep_timeout_ms) {
    s_sleep_requested = true;
    board.softwareShutdown();
  }

  ui_process_screenshot();
}

uint32_t lvHelperGetBacklightTimeoutMs() { return s_backlight_timeout_ms; }

uint32_t lvHelperGetSleepTimeoutMs() { return s_sleep_timeout_ms; }

void lvHelperSetBacklightTimeoutMs(uint32_t timeout_ms) {
  s_backlight_timeout_ms = clamp_timeout(timeout_ms);
  save_settings();
}

void lvHelperSetSleepTimeoutMs(uint32_t timeout_ms) {
  s_sleep_timeout_ms = clamp_timeout(timeout_ms);
}

void lvHelperFormatTimeout(char *out, size_t len, uint32_t timeout_ms) {
  if (!out || len == 0) {
    return;
  }
  if (timeout_ms == 0) {
    snprintf(out, len, "Off");
    return;
  }
  if (timeout_ms < 60000) {
    snprintf(out, len, "%lus", static_cast<unsigned long>(timeout_ms / 1000));
    return;
  }
  snprintf(out, len, "%lum", static_cast<unsigned long>(timeout_ms / 60000));
}
