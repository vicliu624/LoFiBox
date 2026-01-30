#include "ui/screens/now_playing/now_playing_components.h"

#include "ui/screens/now_playing/now_playing_input.h"
#include "ui/screens/now_playing/now_playing_layout.h"
#include "ui/screens/now_playing/now_playing_styles.h"

namespace lofi::ui::screens::now_playing
{
namespace
{
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

}

void build(UiScreen& screen)
{
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
            lv_label_set_text(screen.view.now.title, track.title.length() ? track.title.c_str() : "Unknown Title");
            lv_label_set_text(screen.view.now.artist, track.artist.length() ? track.artist.c_str() : "Unknown Artist");
            lv_label_set_text(screen.view.now.album, track.album.length() ? track.album.c_str() : "Unknown Album");
        } else {
            lv_label_set_text(screen.view.now.title, "No Track");
            lv_label_set_text(screen.view.now.artist, "");
            lv_label_set_text(screen.view.now.album, "");
        }
    }

    if (screen.player && screen.view.now.cover &&
        screen.state.last_cover_version != screen.player->cover_version) {
        screen.state.last_cover_version = screen.player->cover_version;
        if (screen.player->cover_ready && screen.player->cover_path.length() > 0) {
            lv_img_set_src(screen.view.now.cover, screen.player->cover_path.c_str());
            lv_img_set_zoom(screen.view.now.cover, 256);
            lv_image_header_t header{};
            if (lv_image_decoder_get_info(screen.player->cover_path.c_str(), &header) == LV_RES_OK &&
                header.w > 0 && header.h > 0 && screen.view.now.cover_size > 0) {
                uint32_t zx = (static_cast<uint32_t>(screen.view.now.cover_size) * 256U) / header.w;
                uint32_t zy = (static_cast<uint32_t>(screen.view.now.cover_size) * 256U) / header.h;
                uint32_t zoom = zx < zy ? zx : zy;
                if (zoom == 0) {
                    zoom = 256;
                }
                lv_img_set_zoom(screen.view.now.cover, zoom);
            }
            lv_obj_clear_flag(screen.view.now.cover, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(screen.view.now.cover, LV_OBJ_FLAG_HIDDEN);
        }
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
    lv_coord_t knob_w = lv_obj_get_width(screen.view.now.knob);
    if (knob_w < 1) {
        knob_w = 4;
    }
    lv_coord_t knob_x = (screen.view.now.bar_width - knob_w) * percent / 100;
    lv_obj_align(screen.view.now.knob, LV_ALIGN_LEFT_MID, knob_x, 0);

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
