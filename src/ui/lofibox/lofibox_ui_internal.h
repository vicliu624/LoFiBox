#pragma once

#include <Arduino.h>
#include <lvgl.h>

#include "app/library.h"
#include "app/player.h"
#include "ui/screens/common/shell_layout.h"
#include "ui/screens/eq/eq_layout.h"
#include "ui/screens/list_page/list_page_layout.h"
#include "ui/screens/main_menu/main_menu_layout.h"
#include "ui/screens/now_playing/now_playing_layout.h"

namespace lofi::ui
{
enum class PageId
{
    None = 0,
    MainMenu,
    Music,
    Artists,
    Albums,
    Songs,
    Genres,
    Composers,
    Compilations,
    Playlists,
    PlaylistDetail,
    NowPlaying,
    Settings,
    Eq,
    About,
};

enum class UiIntentKind
{
    None = 0,
    Navigate,
    NavigateBack,
    OpenAlbumsAll,
    OpenSongsAll,
    OpenArtist,
    OpenAlbum,
    OpenGenre,
    OpenComposer,
    OpenPlaylist,
    PlayTrack,
    ToggleShuffle,
    ToggleRepeat,
    OpenEq,
    OpenAbout,
    PrevTrack,
    NextTrack,
    TogglePause,
};

enum class SongContext
{
    All = 0,
    Artist,
    Album,
    Genre,
    Composer,
};

enum class AlbumFilter
{
    All = 0,
    Artist,
};

struct ListItem
{
    String left;
    String right;
    UiIntentKind action = UiIntentKind::None;
    PageId next = PageId::None;
    int value = 0;
    int value2 = 0;
    const lv_image_dsc_t* icon = nullptr;
};

struct UiScreen;

struct UiIntent
{
    UiIntentKind kind = UiIntentKind::None;
    PageId next = PageId::None;
    int value = 0;
    int value2 = 0;
    ListItem* item = nullptr;
};

struct RowMeta
{
    UiScreen* screen = nullptr;
    UiIntent intent{};
    lv_obj_t* row = nullptr;
    lv_obj_t* icon = nullptr;
    lv_obj_t* left_label = nullptr;
    lv_obj_t* right_label = nullptr;
    int item_index = -1;
};

enum class TimerDomain
{
    ScreenGeneral = 0,
    NowPlaying,
};

struct TimerEntry
{
    lv_timer_t* timer = nullptr;
    TimerDomain domain = TimerDomain::ScreenGeneral;
};

struct TimerBag
{
    static constexpr int kMaxTimers = 8;
    TimerEntry entries[kMaxTimers] = {};
    int count = 0;

    void reset();
    lv_timer_t* create(TimerDomain domain, lv_timer_cb_t cb, uint32_t period, void* user_data);
    void clear_domain(TimerDomain domain);
    void clear_all();
};

struct UiState
{
    PageId current = PageId::MainMenu;
    PageId stack[8] = {PageId::None};
    int depth = 0;
    int menu_index = 0;

    SongContext song_context = SongContext::All;
    AlbumFilter album_filter = AlbumFilter::All;

    String selected_artist;
    String selected_album;
    String selected_album_artist;
    String selected_genre;
    String selected_composer;

    int current_playlist = -1;
    int last_track_index = -2;
    uint32_t last_meta_version = 0;
    uint32_t last_cover_version = 0;
    int list_offset = 0;
    int list_selected = 0;
    int eq_selected_band = 0;
};

struct UiView
{
    screens::common::layout::RootLayout root{};
    screens::list_page::layout::ListLayout list{};
    screens::now_playing::layout::NowPlayingLayout now{};
    screens::main_menu::layout::MenuLayout menu{};
    screens::eq::layout::EqLayout eq{};
};

struct UiScreen
{
    bool alive = false;
    app::Library* library = nullptr;
    app::PlayerState* player = nullptr;
    lv_group_t* group = nullptr;

    TimerBag timers{};
    UiState state{};
    UiView view{};

    static constexpr int kMaxItems = app::kMaxTracks;
    ListItem items[kMaxItems] = {};
    int items_count = 0;
    RowMeta rows[kMaxItems] = {};
    int row_count = 0;

    int on_the_go[app::kMaxPlaylistTracks] = {};
    int on_the_go_count = 0;
    int playlist_tracks[app::kMaxPlaylistTracks] = {};
    int playlist_count = 0;

    UiIntent pending_intent{};
    bool has_pending_intent = false;
};

void request_intent(UiScreen* screen, const UiIntent& intent);
bool screen_alive(const UiScreen* screen);

} // namespace lofi::ui
