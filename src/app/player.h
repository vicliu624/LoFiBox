#pragma once

#include <Arduino.h>

#include "app/library.h"

namespace app
{

enum class PlaybackMode
{
    Sequential = 0,
    Shuffle,
    RepeatOne,
};

struct PlayerState
{
    int current_index = -1;
    bool is_playing = false;
    bool paused = false;
    PlaybackMode mode = PlaybackMode::Sequential;
    uint8_t volume = 12;
    String cover_path = "";
    bool cover_ready = false;
    uint32_t cover_version = 0;
    uint32_t meta_version = 0;
    int cover_track_index = -1;
    uint32_t cover_pos = 0;
    uint32_t cover_len = 0;
    CoverFormat cover_format = CoverFormat::Unknown;
};

void player_init(PlayerState& state, Library& lib);
void player_loop(PlayerState& state);
void player_play(PlayerState& state, int track_index);
void player_toggle_pause(PlayerState& state);
void player_next(PlayerState& state);
void player_prev(PlayerState& state);

uint32_t player_current_time();
uint32_t player_duration();

} // namespace app
