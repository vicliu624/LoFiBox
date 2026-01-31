#include "ui/screens/now_playing/now_playing_components.h"

#include "ui/screens/now_playing/now_playing_input.h"
#include "ui/screens/now_playing/now_playing_layout.h"
#include "ui/screens/now_playing/now_playing_styles.h"

#include <Arduino.h>
#include <SD.h>
#include <cstring>
#include "src/libs/tjpgd/tjpgd.h"
#include <pngle.h>

namespace lofi::ui::screens::now_playing
{
namespace
{
#if LV_USE_TJPGD
constexpr size_t kJpegWorkBufSize = 8192;

struct CoverDecodeCtx
{
    File* file = nullptr;
    size_t start = 0;
    size_t size = 0;
    size_t pos = 0;
    uint16_t* buf = nullptr;
    lv_coord_t buf_w = 0;
    lv_coord_t buf_h = 0;
    lv_coord_t offset_x = 0;
    lv_coord_t offset_y = 0;
    int scale_fp = 1024;
};
#endif

void format_time(uint32_t seconds, char* out, size_t len, bool unknown)
{
    if (unknown) {
        snprintf(out, len, "--:--");
        return;
    }
    uint32_t mins = seconds / 60;
    uint32_t secs = seconds % 60;
    snprintf(out, len, "%02lu:%02lu", static_cast<unsigned long>(mins), static_cast<unsigned long>(secs));
}

uint16_t rgb565_from_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return lv_color_to_u16(lv_color_make(r, g, b));
}

uint16_t blend_rgb565(uint16_t fg, uint16_t bg, uint8_t alpha)
{
    if (alpha >= 255) {
        return fg;
    }
    if (alpha == 0) {
        return bg;
    }
    return lv_color_16_16_mix(fg, bg, alpha);
}

bool find_cover_start(File& file, size_t pos, size_t size, size_t& image_pos, app::CoverFormat& fmt)
{
    const uint8_t sig_jpg[2] = {0xFF, 0xD8};
    const uint8_t sig_png[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    const uint8_t sig_bmp[2] = {'B', 'M'};

    image_pos = pos;
    fmt = app::CoverFormat::Unknown;

    if (!file || size == 0) {
        return false;
    }

    constexpr size_t kScanMax = 16384;
    size_t max_scan = (size < kScanMax) ? size : kScanMax;
    size_t scanned = 0;
    uint8_t buf[512] = {};

    file.seek(pos);
    while (scanned < max_scan) {
        size_t to_read = max_scan - scanned;
        if (to_read > sizeof(buf)) {
            to_read = sizeof(buf);
        }
        size_t rd = file.read(buf, to_read);
        if (rd == 0) {
            break;
        }

        for (size_t i = 0; i + 1 < rd; ++i) {
            if (buf[i] == sig_jpg[0] && buf[i + 1] == sig_jpg[1]) {
                image_pos = pos + scanned + i;
                fmt = app::CoverFormat::Jpeg;
                return true;
            }
        }

        for (size_t i = 0; i + sizeof(sig_png) <= rd; ++i) {
            if (memcmp(&buf[i], sig_png, sizeof(sig_png)) == 0) {
                image_pos = pos + scanned + i;
                fmt = app::CoverFormat::Png;
                return true;
            }
        }

        for (size_t i = 0; i + 1 < rd; ++i) {
            if (buf[i] == sig_bmp[0] && buf[i + 1] == sig_bmp[1]) {
                image_pos = pos + scanned + i;
                fmt = app::CoverFormat::Bmp;
                return true;
            }
        }

        scanned += rd;
    }

    return false;
}

void clear_cover_buffer(layout::NowPlayingLayout& view, uint16_t color)
{
    if (!view.cover_buf || view.cover_size <= 0) {
        return;
    }
    size_t count = static_cast<size_t>(view.cover_size) * static_cast<size_t>(view.cover_size);
    for (size_t i = 0; i < count; ++i) {
        view.cover_buf[i] = color;
    }
}

#if LV_USE_TJPGD
size_t cover_input(JDEC* jd, uint8_t* buf, size_t len)
{
    auto* ctx = static_cast<CoverDecodeCtx*>(jd ? jd->device : nullptr);
    if (!ctx || !ctx->file || !(*ctx->file)) {
        return 0;
    }
    if (ctx->pos >= ctx->size) {
        return 0;
    }
    size_t remaining = ctx->size - ctx->pos;
    if (len > remaining) {
        len = remaining;
    }

    if (!buf) {
        ctx->pos += len;
        ctx->file->seek(ctx->start + ctx->pos);
        return len;
    }

    size_t rd = ctx->file->read(buf, len);
    ctx->pos += rd;
    return rd;
}

int cover_output(JDEC* jd, void* data, JRECT* rect)
{
    auto* ctx = static_cast<CoverDecodeCtx*>(jd ? jd->device : nullptr);
    if (!ctx || !data || !rect || !ctx->buf) {
        return 0;
    }
    const uint8_t* src = static_cast<const uint8_t*>(data);
    int rect_w = static_cast<int>(rect->right - rect->left + 1);
    int rect_h = static_cast<int>(rect->bottom - rect->top + 1);

    for (int y = 0; y < rect_h; ++y) {
        for (int x = 0; x < rect_w; ++x) {
            uint8_t b = *src++;
            uint8_t g = *src++;
            uint8_t r = *src++;
            int src_x = static_cast<int>(rect->left) + x;
            int src_y = static_cast<int>(rect->top) + y;
            int dst_x = (src_x * 1024) / ctx->scale_fp - ctx->offset_x;
            int dst_y = (src_y * 1024) / ctx->scale_fp - ctx->offset_y;
            if (dst_x < 0 || dst_y < 0 || dst_x >= ctx->buf_w || dst_y >= ctx->buf_h) {
                continue;
            }
            ctx->buf[dst_y * ctx->buf_w + dst_x] = rgb565_from_rgb(r, g, b);
        }
    }
    return 1;
}

bool decode_cover_jpeg(layout::NowPlayingLayout& view, File& file, size_t pos, size_t len)
{
    if (!view.cover_buf || view.cover_size <= 0 || !file || len == 0) {
        return false;
    }

    uint8_t* work = static_cast<uint8_t*>(lv_malloc(kJpegWorkBufSize));
    if (!work) {
        return false;
    }

    CoverDecodeCtx ctx{};
    ctx.file = &file;
    ctx.start = pos;
    ctx.size = len;
    ctx.pos = 0;
    ctx.buf = view.cover_buf;
    ctx.buf_w = view.cover_size;
    ctx.buf_h = view.cover_size;

    file.seek(pos);
    JDEC jd{};
    JRESULT rc = jd_prepare(&jd, cover_input, work, kJpegWorkBufSize, &ctx);
    if (rc != JDR_OK) {
        Serial.printf("[COVER] jpeg prepare fail rc=%d pos=%u len=%u\n",
                      static_cast<int>(rc),
                      static_cast<unsigned>(pos),
                      static_cast<unsigned>(len));
        lv_free(work);
        return false;
    }

    uint8_t scale = 0;
    int scale_fp = 1024;
    int ratio_w = (jd.width > 0 && view.cover_size > 0)
                      ? static_cast<int>((static_cast<int64_t>(jd.width) * 1024) / view.cover_size)
                      : 1024;
    int ratio_h = (jd.height > 0 && view.cover_size > 0)
                      ? static_cast<int>((static_cast<int64_t>(jd.height) * 1024) / view.cover_size)
                      : 1024;
    scale_fp = (ratio_w < ratio_h) ? ratio_w : ratio_h;
    if (scale_fp <= 0) {
        scale_fp = 1024;
    }
#if JD_USE_SCALE == 0
    scale = 0;
#endif
    ctx.scale_fp = scale_fp;
    lv_coord_t dec_w = static_cast<lv_coord_t>((static_cast<int64_t>(jd.width) * 1024 + scale_fp - 1) / scale_fp);
    lv_coord_t dec_h = static_cast<lv_coord_t>((static_cast<int64_t>(jd.height) * 1024 + scale_fp - 1) / scale_fp);
    if (dec_w < 1) {
        dec_w = 1;
    }
    if (dec_h < 1) {
        dec_h = 1;
    }
    ctx.offset_x = (dec_w - view.cover_size) / 2;
    ctx.offset_y = (dec_h - view.cover_size) / 2;
    if (ctx.offset_x < 0) {
        ctx.offset_x = 0;
    }
    if (ctx.offset_y < 0) {
        ctx.offset_y = 0;
    }

    rc = jd_decomp(&jd, cover_output, scale);
    if (rc != JDR_OK) {
        Serial.printf("[COVER] jpeg decomp fail rc=%d pos=%u len=%u\n",
                      static_cast<int>(rc),
                      static_cast<unsigned>(pos),
                      static_cast<unsigned>(len));
    }
    lv_free(work);
    return (rc == JDR_OK);
}
#else
bool decode_cover_jpeg(layout::NowPlayingLayout& view, File& file, size_t pos, size_t len)
{
    (void)view;
    (void)file;
    (void)pos;
    (void)len;
    return false;
}
#endif

struct CoverScale
{
    int scale = 1024;
    int dst_w = 0;
    int dst_h = 0;
    int offset_x = 0;
    int offset_y = 0;
};

CoverScale compute_cover_scale(int src_w, int src_h, int dst_size)
{
    CoverScale out{};
    if (src_w <= 0 || src_h <= 0 || dst_size <= 0) {
        return out;
    }
    int scale_x = (dst_size * 1024) / src_w;
    int scale_y = (dst_size * 1024) / src_h;
    out.scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (out.scale <= 0) {
        out.scale = 1;
    }
    out.dst_w = (src_w * out.scale + 1023) / 1024;
    out.dst_h = (src_h * out.scale + 1023) / 1024;
    out.offset_x = (dst_size - out.dst_w) / 2;
    out.offset_y = (dst_size - out.dst_h) / 2;
    return out;
}

bool decode_cover_bmp(layout::NowPlayingLayout& view, File& file, size_t pos, size_t len)
{
    if (!view.cover_buf || view.cover_size <= 0 || !file || len < 54) {
        return false;
    }

    uint8_t header[54] = {};
    file.seek(pos);
    if (file.read(header, sizeof(header)) != sizeof(header)) {
        return false;
    }
    if (header[0] != 'B' || header[1] != 'M') {
        return false;
    }

    uint32_t off_bits = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
    int32_t width = static_cast<int32_t>(header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24));
    int32_t height = static_cast<int32_t>(header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24));
    uint16_t bpp = static_cast<uint16_t>(header[28] | (header[29] << 8));
    uint32_t compression =
        header[30] | (header[31] << 8) | (header[32] << 16) | (header[33] << 24);

    if (width <= 0 || height == 0) {
        return false;
    }
    if (compression != 0) {
        return false;
    }
    bool top_down = false;
    if (height < 0) {
        top_down = true;
        height = -height;
    }
    if (bpp != 16 && bpp != 24 && bpp != 32) {
        return false;
    }

    uint32_t row_size = ((static_cast<uint32_t>(bpp) * static_cast<uint32_t>(width) + 31) / 32) * 4;
    if (off_bits + static_cast<uint32_t>(height) * row_size > len) {
        return false;
    }

    CoverScale scale = compute_cover_scale(width, height, view.cover_size);
    uint16_t bg = rgb565_from_rgb(0x0a, 0x0b, 0x0e);

    uint8_t* row = static_cast<uint8_t*>(lv_malloc(row_size));
    if (!row) {
        return false;
    }

    for (int y = 0; y < height; ++y) {
        int file_row = top_down ? y : (height - 1 - y);
        uint32_t row_pos = static_cast<uint32_t>(pos + off_bits + static_cast<uint32_t>(file_row) * row_size);
        file.seek(row_pos);
        if (file.read(row, row_size) != row_size) {
            lv_free(row);
            return false;
        }
        int dst_y = scale.offset_y + (y * scale.scale) / 1024;
        if (dst_y < 0 || dst_y >= view.cover_size) {
            continue;
        }
        for (int x = 0; x < width; ++x) {
            int dst_x = scale.offset_x + (x * scale.scale) / 1024;
            if (dst_x < 0 || dst_x >= view.cover_size) {
                continue;
            }
            uint8_t b = 0;
            uint8_t g = 0;
            uint8_t r = 0;
            uint8_t a = 255;
            if (bpp == 24) {
                size_t off = static_cast<size_t>(x) * 3;
                b = row[off + 0];
                g = row[off + 1];
                r = row[off + 2];
            } else if (bpp == 32) {
                size_t off = static_cast<size_t>(x) * 4;
                b = row[off + 0];
                g = row[off + 1];
                r = row[off + 2];
                a = row[off + 3];
            } else if (bpp == 16) {
                size_t off = static_cast<size_t>(x) * 2;
                uint16_t v = static_cast<uint16_t>(row[off] | (row[off + 1] << 8));
                uint8_t rr = ((v >> 11) & 0x1F) << 3;
                uint8_t gg = ((v >> 5) & 0x3F) << 2;
                uint8_t bb = (v & 0x1F) << 3;
                r = rr;
                g = gg;
                b = bb;
            }
            uint16_t fg = rgb565_from_rgb(r, g, b);
            view.cover_buf[dst_y * view.cover_size + dst_x] = blend_rgb565(fg, bg, a);
        }
    }

    lv_free(row);
    return true;
}

struct PngleCtx
{
    layout::NowPlayingLayout* view = nullptr;
    int scale = 1024;
    int offset_x = 0;
    int offset_y = 0;
    uint16_t bg = 0;
    bool scale_ready = false;
};

static PngleCtx* s_pngle_ctx = nullptr;

static void pngle_draw(pngle_t* png, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint8_t rgba[4])
{
    (void)png;
    if (!s_pngle_ctx || !s_pngle_ctx->view || !s_pngle_ctx->view->cover_buf) {
        return;
    }
    if (!s_pngle_ctx->scale_ready) {
        CoverScale scale = compute_cover_scale(static_cast<int>(w), static_cast<int>(h),
                                               s_pngle_ctx->view->cover_size);
        s_pngle_ctx->scale = scale.scale;
        s_pngle_ctx->offset_x = scale.offset_x;
        s_pngle_ctx->offset_y = scale.offset_y;
        s_pngle_ctx->scale_ready = true;
    }
    int dst_x = s_pngle_ctx->offset_x + (static_cast<int>(x) * s_pngle_ctx->scale) / 1024;
    int dst_y = s_pngle_ctx->offset_y + (static_cast<int>(y) * s_pngle_ctx->scale) / 1024;
    if (dst_x < 0 || dst_y < 0 ||
        dst_x >= s_pngle_ctx->view->cover_size ||
        dst_y >= s_pngle_ctx->view->cover_size) {
        return;
    }
    uint16_t fg = rgb565_from_rgb(rgba[0], rgba[1], rgba[2]);
    s_pngle_ctx->view->cover_buf[dst_y * s_pngle_ctx->view->cover_size + dst_x] =
        blend_rgb565(fg, s_pngle_ctx->bg, rgba[3]);
}

bool decode_cover_png(layout::NowPlayingLayout& view, File& file, size_t pos, size_t len)
{
    if (!view.cover_buf || view.cover_size <= 0 || !file || len == 0) {
        return false;
    }

    pngle_t* png = pngle_new();
    if (!png) {
        return false;
    }
    PngleCtx ctx{};
    ctx.view = &view;
    ctx.bg = rgb565_from_rgb(0x0a, 0x0b, 0x0e);
    s_pngle_ctx = &ctx;
    pngle_set_draw_callback(png, pngle_draw);

    file.seek(pos);
    uint8_t buf[512] = {};
    size_t remaining = len;
    size_t cached = 0;
    bool ok = true;

    while (remaining > 0) {
        size_t to_read = sizeof(buf) - cached;
        if (to_read > remaining) {
            to_read = remaining;
        }
        size_t rd = file.read(buf + cached, to_read);
        if (rd == 0) {
            ok = false;
            break;
        }
        remaining -= rd;
        size_t feed_len = cached + rd;
        int fed = pngle_feed(png, buf, static_cast<int>(feed_len));
        if (fed < 0) {
            ok = false;
            break;
        }
        if (static_cast<size_t>(fed) > feed_len) {
            ok = false;
            break;
        }
        cached = feed_len - static_cast<size_t>(fed);
        if (cached > 0) {
            memmove(buf, buf + fed, cached);
        }
        if (fed == 0 && cached == sizeof(buf)) {
            ok = false;
            break;
        }
    }

    if (ok && cached > 0) {
        int fed = pngle_feed(png, buf, static_cast<int>(cached));
        if (fed < 0) {
            ok = false;
        }
    }

    pngle_destroy(png);
    s_pngle_ctx = nullptr;
    return ok;
}

void update_cover(UiScreen& screen)
{
    if (!screen.player || !screen.library) {
        return;
    }
    auto& view = screen.view.now;
    if (!view.cover || !view.cover_buf) {
        return;
    }

    uint16_t bg = rgb565_from_rgb(0x0a, 0x0b, 0x0e);
    if (!screen.player->cover_ready ||
        (screen.player->cover_format != app::CoverFormat::Jpeg &&
         screen.player->cover_format != app::CoverFormat::Png &&
         screen.player->cover_format != app::CoverFormat::Bmp) ||
        screen.player->cover_len == 0 ||
        screen.player->current_index < 0 ||
        screen.player->current_index >= screen.library->track_count ||
        screen.player->cover_track_index != screen.player->current_index) {
        clear_cover_buffer(view, bg);
        lv_obj_invalidate(view.cover);
        lv_obj_add_flag(view.cover, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    const app::TrackInfo& track = screen.library->tracks[screen.player->current_index];
    File f = SD.open(track.path ? track.path : "", FILE_READ);
    if (!f) {
        clear_cover_buffer(view, bg);
        lv_obj_invalidate(view.cover);
        lv_obj_add_flag(view.cover, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    clear_cover_buffer(view, bg);
    size_t image_pos = screen.player->cover_pos;
    size_t image_len = screen.player->cover_len;
    app::CoverFormat fmt = screen.player->cover_format;

    if (fmt == app::CoverFormat::Unknown) {
        size_t found_pos = 0;
        app::CoverFormat found_fmt = app::CoverFormat::Unknown;
        if (find_cover_start(f, screen.player->cover_pos, screen.player->cover_len, found_pos, found_fmt)) {
            image_pos = found_pos;
            image_len = screen.player->cover_len - (found_pos - screen.player->cover_pos);
            fmt = found_fmt;
        }
    }

    bool ok = false;
    switch (fmt) {
    case app::CoverFormat::Jpeg:
        ok = decode_cover_jpeg(view, f, image_pos, image_len);
        break;
    case app::CoverFormat::Png:
        ok = decode_cover_png(view, f, image_pos, image_len);
        break;
    case app::CoverFormat::Bmp:
        ok = decode_cover_bmp(view, f, image_pos, image_len);
        break;
    default:
        ok = false;
        break;
    }
    f.close();
    lv_obj_invalidate(view.cover);

    if (ok) {
        lv_obj_clear_flag(view.cover, LV_OBJ_FLAG_HIDDEN);
    } else {
        Serial.printf("[COVER] decode failed fmt=%d pos=%u len=%u\n",
                      static_cast<int>(fmt),
                      static_cast<unsigned>(image_pos),
                      static_cast<unsigned>(image_len));
        lv_obj_add_flag(view.cover, LV_OBJ_FLAG_HIDDEN);
    }
}

}

void build(UiScreen& screen)
{
    screen.state.last_track_index = -2;
    screen.state.last_meta_version = 0xFFFFFFFFu;
    screen.state.last_cover_version = 0xFFFFFFFFu;

    if (screen.view.now.cover_buf) {
        lv_free(screen.view.now.cover_buf);
        screen.view.now.cover_buf = nullptr;
        screen.view.now.cover_buf_size = 0;
    }

    styles::init_once();
    styles::apply_content(screen.view.root.content);
    screen.view.now = layout::create_now_playing(screen.view.root.content);

    styles::apply_cover(screen.view.now.cover);
    styles::apply_title(screen.view.now.title);
    styles::apply_subtitle(screen.view.now.artist);
    styles::apply_subtitle(screen.view.now.album);
    styles::apply_time_label(screen.view.now.time_left);
    styles::apply_time_label(screen.view.now.time_right);
    styles::apply_bar_wrap(screen.view.now.bar_wrap);
    styles::apply_bar(screen.view.now.bar);
    styles::apply_knob(screen.view.now.knob);
    styles::apply_controls_row(screen.view.now.controls_row);
    styles::apply_control_icon(screen.view.now.ctrl_prev);
    styles::apply_control_icon(screen.view.now.ctrl_play);
    styles::apply_control_icon(screen.view.now.ctrl_next);
    styles::apply_control_icon(screen.view.now.ctrl_shuffle);
    styles::apply_control_icon(screen.view.now.ctrl_repeat);
    styles::apply_key_sink(screen.view.now.key_sink);

    lv_label_set_text(screen.view.now.title, "No Track");
    if (screen.view.now.title) {
        lv_label_set_long_mode(screen.view.now.title, LV_LABEL_LONG_SCROLL_CIRCULAR);
        const lv_font_t* font = lv_obj_get_style_text_font(screen.view.now.title, LV_PART_MAIN);
        if (font) {
            lv_coord_t char_w = lv_font_get_glyph_width(font, 'W', 0);
            lv_coord_t max_w = char_w * 30;
            if (max_w > 0) {
                lv_obj_set_width(screen.view.now.title, max_w);
            }
        }
        lv_obj_set_height(screen.view.now.title, LV_SIZE_CONTENT);
    }
    lv_label_set_text(screen.view.now.artist, "");
    lv_label_set_text(screen.view.now.album, "");
    lv_label_set_text(screen.view.now.time_left, "--:--");
    lv_label_set_text(screen.view.now.time_right, "--:--");
    lv_label_set_text(screen.view.now.ctrl_prev, LV_SYMBOL_PREV);
    lv_label_set_text(screen.view.now.ctrl_play, LV_SYMBOL_PLAY);
    lv_label_set_text(screen.view.now.ctrl_next, LV_SYMBOL_NEXT);
    lv_label_set_text(screen.view.now.ctrl_shuffle, LV_SYMBOL_SHUFFLE);
    lv_label_set_text(screen.view.now.ctrl_repeat, LV_SYMBOL_LOOP);
    lv_obj_add_flag(screen.view.now.cover, LV_OBJ_FLAG_HIDDEN);

    if (screen.view.now.cover_size > 0) {
        size_t buf_size = static_cast<size_t>(screen.view.now.cover_size) *
                          static_cast<size_t>(screen.view.now.cover_size) *
                          sizeof(uint16_t);
        screen.view.now.cover_buf = static_cast<uint16_t*>(lv_malloc(buf_size));
        screen.view.now.cover_buf_size = buf_size;
        if (screen.view.now.cover_buf) {
            lv_canvas_set_buffer(screen.view.now.cover, screen.view.now.cover_buf, screen.view.now.cover_size,
                                 screen.view.now.cover_size, LV_COLOR_FORMAT_RGB565);
            clear_cover_buffer(screen.view.now, rgb565_from_rgb(0x0a, 0x0b, 0x0e));
        }
    }

    lv_bar_set_range(screen.view.now.bar, 0, 100);
    lv_bar_set_value(screen.view.now.bar, 0, LV_ANIM_OFF);

    input::attach(screen, screen.view.now.key_sink);
}

void update(UiScreen& screen)
{
    if (screen.state.current != PageId::NowPlaying || !screen.view.now.title) {
        return;
    }

    int idx = (screen.player) ? screen.player->current_index : -1;
    bool has_track = (screen.library && idx >= 0 && idx < screen.library->track_count);
    bool meta_changed = false;
    if (screen.player && screen.state.last_meta_version != screen.player->meta_version) {
        screen.state.last_meta_version = screen.player->meta_version;
        meta_changed = true;
    }

    if (idx != screen.state.last_track_index || meta_changed) {
        screen.state.last_track_index = idx;
        if (has_track) {
            const app::TrackInfo& track = screen.library->tracks[idx];
            String safe_title = (track.title && track.title[0]) ? String(track.title) : String("Unknown Title");
            String safe_artist = (track.artist && track.artist[0]) ? String(track.artist) : String("Unknown Artist");
            String safe_album = (track.album && track.album[0]) ? String(track.album) : String("Unknown Album");
            safe_title.replace('\n', ' ');
            safe_title.replace('\r', ' ');
            safe_artist.replace('\n', ' ');
            safe_artist.replace('\r', ' ');
            safe_album.replace('\n', ' ');
            safe_album.replace('\r', ' ');
            lv_label_set_text(screen.view.now.title, safe_title.c_str());
            lv_label_set_text(screen.view.now.artist, safe_artist.c_str());
            lv_label_set_text(screen.view.now.album, safe_album.c_str());
        } else {
            lv_label_set_text(screen.view.now.title, "No Track");
            lv_label_set_text(screen.view.now.artist, "");
            lv_label_set_text(screen.view.now.album, "");
        }
    }

    if (screen.player && screen.view.now.cover &&
        screen.state.last_cover_version != screen.player->cover_version) {
        screen.state.last_cover_version = screen.player->cover_version;
        update_cover(screen);
    }

    char buf_left[16] = {0};
    char buf_right[16] = {0};
    uint32_t elapsed = has_track ? app::player_current_time() : 0;
    uint32_t duration = has_track ? app::player_duration() : 0;
    format_time(elapsed, buf_left, sizeof(buf_left), !has_track);
    format_time(duration, buf_right, sizeof(buf_right), !has_track || duration == 0);
    lv_label_set_text(screen.view.now.time_left, buf_left);
    lv_label_set_text(screen.view.now.time_right, buf_right);

    int percent = 0;
    if (duration > 0) {
        percent = static_cast<int>((elapsed * 100UL) / duration);
        if (percent < 0) {
            percent = 0;
        }
        if (percent > 100) {
            percent = 100;
        }
    }
    lv_bar_set_value(screen.view.now.bar, percent, LV_ANIM_OFF);
    lv_coord_t bar_w = lv_obj_get_width(screen.view.now.bar);
    if (bar_w < 1) {
        bar_w = screen.view.now.bar_width;
    }
    lv_coord_t knob_w = lv_obj_get_width(screen.view.now.knob);
    if (knob_w < 1) {
        knob_w = 4;
    }
    lv_coord_t knob_x = (bar_w - knob_w) * percent / 100;
    if (percent >= 100) {
        knob_x = bar_w - knob_w;
    }
    lv_obj_align_to(screen.view.now.knob, screen.view.now.bar, LV_ALIGN_LEFT_MID, knob_x, 0);

    bool paused = screen.player ? screen.player->paused : false;
    lv_label_set_text(screen.view.now.ctrl_play, paused ? LV_SYMBOL_PLAY : LV_SYMBOL_PAUSE);
    if (screen.player && screen.player->mode == app::PlaybackMode::Shuffle) {
        lv_obj_add_state(screen.view.now.ctrl_shuffle, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(screen.view.now.ctrl_shuffle, LV_STATE_CHECKED);
    }
    if (screen.player && screen.player->mode == app::PlaybackMode::RepeatOne) {
        lv_obj_add_state(screen.view.now.ctrl_repeat, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(screen.view.now.ctrl_repeat, LV_STATE_CHECKED);
    }
}

} // namespace lofi::ui::screens::now_playing
