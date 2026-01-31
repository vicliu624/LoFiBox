#include "LV_Helper.h"

#include <Arduino.h>
#include <stdlib.h>
#include <SD.h>

#include "board/BoardBase.h"

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 240
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 135
#endif

namespace
{
static lv_display_t* s_display = nullptr;
static lv_color_t* s_buf1 = nullptr;
static lv_color_t* s_buf2 = nullptr;
static lv_indev_t* s_indev_keyboard = nullptr;
static bool s_key_pending = false;
static uint32_t s_last_key = 0;
static lv_fs_drv_t s_sd_drv;
static uint32_t s_last_input_ms = 0;
static bool s_display_sleep = false;
static uint8_t s_saved_brightness = 0;
static uint8_t s_saved_kb_brightness = 0;

constexpr uint32_t kIdleTimeoutMs = 30000;

static void wake_display()
{
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

static void note_input_activity()
{
    s_last_input_ms = millis();
    wake_display();
}

static void disp_flush(lv_display_t* disp_drv, const lv_area_t* area, uint8_t* color_p)
{
    (void)disp_drv;
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;

#if defined(BOARD_TLORA_PAGER)
    lv_draw_sw_rgb565_swap(color_p, w * h);
#endif

    board.displayPushColors(area->x1, area->y1, w, h, reinterpret_cast<uint16_t*>(color_p));
    lv_display_flush_ready(disp_drv);
}

static uint32_t lv_tick_get_callback()
{
    return millis();
}

static void keypad_read(lv_indev_t* drv, lv_indev_data_t* data)
{
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
        data->key = key;
        data->state = LV_INDEV_STATE_PRESSED;
        s_last_key = key;
        s_key_pending = true;
        return;
    }

    data->state = LV_INDEV_STATE_RELEASED;
}

static bool sd_ready_cb(lv_fs_drv_t* drv)
{
    (void)drv;
    return board.isSDReady();
}

static void* sd_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode)
{
    (void)drv;
    const char* fs_path = path && path[0] ? path : "/";
    const char* open_mode = (mode & LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ;
    File file = SD.open(fs_path, open_mode);
    if (!file) {
        return nullptr;
    }
    File* handle = new File(file);
    return handle;
}

static lv_fs_res_t sd_close_cb(lv_fs_drv_t* drv, void* file_p)
{
    (void)drv;
    File* handle = static_cast<File*>(file_p);
    if (!handle) {
        return LV_FS_RES_INV_PARAM;
    }
    handle->close();
    delete handle;
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_read_cb(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br)
{
    (void)drv;
    File* handle = static_cast<File*>(file_p);
    if (!handle) {
        return LV_FS_RES_INV_PARAM;
    }
    size_t read_bytes = handle->read(static_cast<uint8_t*>(buf), btr);
    if (br) {
        *br = static_cast<uint32_t>(read_bytes);
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_write_cb(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw)
{
    (void)drv;
    File* handle = static_cast<File*>(file_p);
    if (!handle) {
        return LV_FS_RES_INV_PARAM;
    }
    size_t written = handle->write(static_cast<const uint8_t*>(buf), btw);
    if (bw) {
        *bw = static_cast<uint32_t>(written);
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_seek_cb(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence)
{
    (void)drv;
    File* handle = static_cast<File*>(file_p);
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

static lv_fs_res_t sd_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p)
{
    (void)drv;
    File* handle = static_cast<File*>(file_p);
    if (!handle || !pos_p) {
        return LV_FS_RES_INV_PARAM;
    }
    *pos_p = static_cast<uint32_t>(handle->position());
    return LV_FS_RES_OK;
}
} // namespace

void beginLvglHelper()
{
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

    s_buf1 = static_cast<lv_color_t*>(malloc(buffer_size));
    s_buf2 = static_cast<lv_color_t*>(malloc(buffer_size));

    s_display = lv_display_create(width, height);
    lv_display_set_buffers(s_display, s_buf1, s_buf2, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
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

void lvHelperTick()
{
    uint32_t now = millis();
    if (!s_display_sleep && static_cast<uint32_t>(now - s_last_input_ms) >= kIdleTimeoutMs) {
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
}
