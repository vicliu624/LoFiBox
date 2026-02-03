#include <Arduino.h>
#include <SD.h>
#include <lvgl.h>
#include <time.h>

#include "ui/LV_Helper.h"

#if LV_USE_SNAPSHOT
extern "C" lv_draw_buf_t *lv_snapshot_take(lv_obj_t *obj, lv_color_format_t cf);
extern "C" void lv_draw_buf_destroy(lv_draw_buf_t *draw_buf);
#endif

extern "C" void ui_ime_toggle_mode() {}
extern "C" bool ui_ime_is_active() { return false; }

namespace {
static bool s_screenshot_pending = false;

void write_le16(File &file, uint16_t value) {
  uint8_t bytes[2] = {static_cast<uint8_t>(value & 0xFF),
                      static_cast<uint8_t>((value >> 8) & 0xFF)};
  file.write(bytes, sizeof(bytes));
}

void write_le32(File &file, uint32_t value) {
  uint8_t bytes[4] = {static_cast<uint8_t>(value & 0xFF),
                      static_cast<uint8_t>((value >> 8) & 0xFF),
                      static_cast<uint8_t>((value >> 16) & 0xFF),
                      static_cast<uint8_t>((value >> 24) & 0xFF)};
  file.write(bytes, sizeof(bytes));
}

bool write_bmp_rgb888(File &file, const uint8_t *pixels, uint32_t width,
                      uint32_t height, uint32_t stride_bytes) {
  if (!pixels || width == 0 || height == 0) {
    return false;
  }
  if (stride_bytes == 0) {
    stride_bytes = width * 2;
  }

  const uint32_t row_bytes = width * 3;
  const uint32_t row_padded = (row_bytes + 3) & ~3u;
  const uint32_t pixel_data_size = row_padded * height;
  const uint32_t header_size = 14 + 40;
  const uint32_t file_size = header_size + pixel_data_size;

  static const uint8_t sig[2] = {'B', 'M'};
  file.write(sig, sizeof(sig));
  write_le32(file, file_size);
  write_le16(file, 0);
  write_le16(file, 0);
  write_le32(file, header_size);

  write_le32(file, 40);
  write_le32(file, width);
  write_le32(file, height);
  write_le16(file, 1);
  write_le16(file, 24);
  write_le32(file, 0);
  write_le32(file, pixel_data_size);
  write_le32(file, 0);
  write_le32(file, 0);
  write_le32(file, 0);
  write_le32(file, 0);

  uint8_t *rowbuf = static_cast<uint8_t *>(malloc(row_padded));
  if (!rowbuf) {
    return false;
  }
  memset(rowbuf, 0, row_padded);

  for (uint32_t y = 0; y < height; ++y) {
    const uint16_t *row = reinterpret_cast<const uint16_t *>(
        pixels + (height - 1 - y) * stride_bytes);
    size_t idx = 0;
    for (uint32_t x = 0; x < width; ++x) {
      uint16_t px = row[x];
      uint8_t r5 = static_cast<uint8_t>((px >> 11) & 0x1F);
      uint8_t g6 = static_cast<uint8_t>((px >> 5) & 0x3F);
      uint8_t b5 = static_cast<uint8_t>(px & 0x1F);
      uint8_t r = static_cast<uint8_t>((r5 << 3) | (r5 >> 2));
      uint8_t g = static_cast<uint8_t>((g6 << 2) | (g6 >> 4));
      uint8_t b = static_cast<uint8_t>((b5 << 3) | (b5 >> 2));
      rowbuf[idx++] = b;
      rowbuf[idx++] = g;
      rowbuf[idx++] = r;
    }
    if (row_padded > row_bytes) {
      memset(rowbuf + row_bytes, 0, row_padded - row_bytes);
    }
    file.write(rowbuf, row_padded);
  }

  free(rowbuf);

  return true;
}

void format_screenshot_name(char *out, size_t len) {
  time_t now = time(nullptr);
  struct tm t = {};
  if (now > 0 && localtime_r(&now, &t)) {
    snprintf(out, len, "/screen/screenshot_%04d%02d%02d_%02d%02d%02d.bmp",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
             t.tm_sec);
    return;
  }

  uint32_t seconds = millis() / 1000;
  uint32_t sec = seconds % 60;
  uint32_t min = (seconds / 60) % 60;
  uint32_t hour = (seconds / 3600) % 24;
  snprintf(out, len, "/screen/screenshot_19700101_%02u%02u%02u.bmp",
           static_cast<unsigned>(hour), static_cast<unsigned>(min),
           static_cast<unsigned>(sec));
}

bool capture_screenshot_to_sd() {
#if LV_USE_SNAPSHOT
  if (SD.cardType() == CARD_NONE) {
    return false;
  }

  if (!SD.exists("/screen")) {
    if (!SD.mkdir("/screen")) {
      return false;
    }
  }

  lv_obj_t *screen = lv_screen_active();
  if (!screen) {
    return false;
  }

  lv_draw_buf_t *snap = lv_snapshot_take(screen, LV_COLOR_FORMAT_RGB565);
  if (!snap) {
    return false;
  }

  uint32_t width = snap->header.w;
  uint32_t height = snap->header.h;
  uint32_t stride = snap->header.stride;
  if (stride == 0) {
    stride = lv_draw_buf_width_to_stride(width, LV_COLOR_FORMAT_RGB565);
  }

  char filename[64] = {};
  format_screenshot_name(filename, sizeof(filename));

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    lv_draw_buf_destroy(snap);
    return false;
  }

  bool ok = write_bmp_rgb888(file, snap->data, width, height, stride);
  file.close();
  lv_draw_buf_destroy(snap);
  return ok;
#else
  return false;
#endif
}
} // namespace

bool ui_take_screenshot_to_sd() {
  s_screenshot_pending = true;
  return true;
}

bool ui_process_screenshot() {
  if (!s_screenshot_pending) {
    return false;
  }
  s_screenshot_pending = false;
  return capture_screenshot_to_sd();
}
