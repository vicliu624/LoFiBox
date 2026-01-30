#include "ui/lofibox/lofibox_components.h"

#include "board/BoardBase.h"
#include "ui/screens/about/about_components.h"
#include "ui/screens/about/about_input.h"
#include "ui/screens/about/about_layout.h"
#include "ui/screens/about/about_styles.h"
#include "ui/screens/albums/albums_components.h"
#include "ui/screens/albums/albums_input.h"
#include "ui/screens/albums/albums_layout.h"
#include "ui/screens/albums/albums_styles.h"
#include "ui/screens/artists/artists_components.h"
#include "ui/screens/artists/artists_input.h"
#include "ui/screens/artists/artists_layout.h"
#include "ui/screens/artists/artists_styles.h"
#include "ui/screens/common/shell_layout.h"
#include "ui/screens/common/shell_styles.h"
#include "ui/screens/compilations/compilations_components.h"
#include "ui/screens/compilations/compilations_input.h"
#include "ui/screens/compilations/compilations_layout.h"
#include "ui/screens/compilations/compilations_styles.h"
#include "ui/screens/composers/composers_components.h"
#include "ui/screens/composers/composers_input.h"
#include "ui/screens/composers/composers_layout.h"
#include "ui/screens/composers/composers_styles.h"
#include "ui/screens/eq/eq_components.h"
#include "ui/screens/eq/eq_input.h"
#include "ui/screens/eq/eq_layout.h"
#include "ui/screens/eq/eq_styles.h"
#include "ui/screens/genres/genres_components.h"
#include "ui/screens/genres/genres_input.h"
#include "ui/screens/genres/genres_layout.h"
#include "ui/screens/genres/genres_styles.h"
#include "ui/screens/list_page/list_page_layout.h"
#include "ui/screens/main_menu/main_menu_components.h"
#include "ui/screens/main_menu/main_menu_input.h"
#include "ui/screens/main_menu/main_menu_layout.h"
#include "ui/screens/main_menu/main_menu_styles.h"
#include "ui/screens/music/music_components.h"
#include "ui/screens/music/music_input.h"
#include "ui/screens/music/music_layout.h"
#include "ui/screens/music/music_styles.h"
#include "ui/screens/now_playing/now_playing_components.h"
#include "ui/screens/playlist_detail/playlist_detail_components.h"
#include "ui/screens/playlist_detail/playlist_detail_input.h"
#include "ui/screens/playlist_detail/playlist_detail_layout.h"
#include "ui/screens/playlist_detail/playlist_detail_styles.h"
#include "ui/screens/playlists/playlists_components.h"
#include "ui/screens/playlists/playlists_input.h"
#include "ui/screens/playlists/playlists_layout.h"
#include "ui/screens/playlists/playlists_styles.h"
#include "ui/screens/settings/settings_components.h"
#include "ui/screens/settings/settings_input.h"
#include "ui/screens/settings/settings_layout.h"
#include "ui/screens/settings/settings_styles.h"
#include "ui/screens/songs/songs_components.h"
#include "ui/screens/songs/songs_input.h"
#include "ui/screens/songs/songs_layout.h"
#include "ui/screens/songs/songs_styles.h"
#include "ui/ui_common.h"
#include <SD.h>
#include "app/library.h"

namespace lofi::ui::components
{
namespace
{
namespace shell_layout = screens::common::layout;
namespace shell_styles = screens::common::styles;
namespace list_layout = screens::list_page::layout;

bool page_needs_library(PageId id)
{
    switch (id) {
    case PageId::Music:
    case PageId::Artists:
    case PageId::Albums:
    case PageId::Songs:
    case PageId::Genres:
    case PageId::Composers:
    case PageId::Compilations:
    case PageId::Playlists:
    case PageId::PlaylistDetail:
        return true;
    default:
        return false;
    }
}

void scan_tick()
{
    lv_timer_handler();
    yield();
}

void ensure_library_scanned(UiScreen& screen)
{
    if (!screen.library || screen.library->scanned) {
        return;
    }
    if (!board.isSDReady()) {
        return;
    }
    app::library_scan(*screen.library, SD, "/music", 3, 256, false, scan_tick);
}

struct ListPageApi
{
    void (*init_styles)() = nullptr;
    void (*apply_content)(lv_obj_t*) = nullptr;
    void (*apply_list)(lv_obj_t*) = nullptr;
    void (*apply_row)(lv_obj_t*) = nullptr;
    void (*apply_left)(lv_obj_t*) = nullptr;
    void (*apply_right)(lv_obj_t*) = nullptr;
    list_layout::ListLayout (*create_list)(lv_obj_t*) = nullptr;
    list_layout::ListRowLayout (*create_row)(lv_obj_t*) = nullptr;
    void (*attach_row)(UiScreen&, RowMeta&) = nullptr;
    void (*focus_first)(lv_group_t*, lv_obj_t*) = nullptr;
};

inline ListPageApi make_list_api(void (*init_styles)(), void (*apply_content)(lv_obj_t*), void (*apply_list)(lv_obj_t*),
                                 void (*apply_row)(lv_obj_t*), void (*apply_left)(lv_obj_t*),
                                 void (*apply_right)(lv_obj_t*), list_layout::ListLayout (*create_list)(lv_obj_t*),
                                 list_layout::ListRowLayout (*create_row)(lv_obj_t*),
                                 void (*attach_row)(UiScreen&, RowMeta&), void (*focus_first)(lv_group_t*, lv_obj_t*))
{
    ListPageApi api{};
    api.init_styles = init_styles;
    api.apply_content = apply_content;
    api.apply_list = apply_list;
    api.apply_row = apply_row;
    api.apply_left = apply_left;
    api.apply_right = apply_right;
    api.create_list = create_list;
    api.create_row = create_row;
    api.attach_row = attach_row;
    api.focus_first = focus_first;
    return api;
}

#define LIST_API(NS)                                                                                                   \
    make_list_api(NS::styles::init_once, NS::styles::apply_content, NS::styles::apply_list, NS::styles::apply_list_row, \
                  NS::styles::apply_list_label_left, NS::styles::apply_list_label_right, NS::layout::create_list,      \
                  NS::layout::create_list_row, NS::input::attach_row, NS::input::focus_first)

ListPageApi list_api_for(PageId id)
{
    switch (id) {
    case PageId::Music:
        return LIST_API(screens::music);
    case PageId::Artists:
        return LIST_API(screens::artists);
    case PageId::Albums:
        return LIST_API(screens::albums);
    case PageId::Songs:
        return LIST_API(screens::songs);
    case PageId::Genres:
        return LIST_API(screens::genres);
    case PageId::Composers:
        return LIST_API(screens::composers);
    case PageId::Compilations:
        return LIST_API(screens::compilations);
    case PageId::Playlists:
        return LIST_API(screens::playlists);
    case PageId::PlaylistDetail:
        return LIST_API(screens::playlist_detail);
    case PageId::Settings:
        return LIST_API(screens::settings);
    case PageId::Eq:
        return LIST_API(screens::eq);
    case PageId::About:
        return LIST_API(screens::about);
    default:
        return LIST_API(screens::music);
    }
}

#undef LIST_API

static const char* kPlaylistNames[] = {
    "On-The-Go",
    "Recently Added",
    "Most Played",
    "Recently Played",
};

const char* page_title(PageId id)
{
    switch (id) {
    case PageId::MainMenu:
        return "Menu";
    case PageId::Music:
        return "Music";
    case PageId::Artists:
        return "Artists";
    case PageId::Albums:
        return "Albums";
    case PageId::Songs:
        return "Songs";
    case PageId::Genres:
        return "Genres";
    case PageId::Composers:
        return "Composers";
    case PageId::Compilations:
        return "Compilations";
    case PageId::Playlists:
        return "Playlists";
    case PageId::PlaylistDetail:
        return "Playlist";
    case PageId::NowPlaying:
        return "Now Playing";
    case PageId::Settings:
        return "Settings";
    case PageId::Eq:
        return "EQ";
    case PageId::About:
        return "About";
    default:
        return "";
    }
}

const char* current_page_title(const UiScreen& screen)
{
    if (screen.state.current == PageId::PlaylistDetail && screen.state.current_playlist >= 0 &&
        screen.state.current_playlist < static_cast<int>(sizeof(kPlaylistNames) / sizeof(kPlaylistNames[0]))) {
        return kPlaylistNames[screen.state.current_playlist];
    }
    if (screen.state.current == PageId::Albums && screen.state.album_filter == AlbumFilter::Artist &&
        screen.state.selected_artist.length() > 0) {
        return screen.state.selected_artist.c_str();
    }
    if (screen.state.current == PageId::Songs) {
        switch (screen.state.song_context) {
        case SongContext::Album:
            if (screen.state.selected_album.length() > 0) {
                return screen.state.selected_album.c_str();
            }
            break;
        case SongContext::Artist:
            if (screen.state.selected_artist.length() > 0) {
                return screen.state.selected_artist.c_str();
            }
            break;
        case SongContext::Genre:
            if (screen.state.selected_genre.length() > 0) {
                return screen.state.selected_genre.c_str();
            }
            break;
        case SongContext::Composer:
            if (screen.state.selected_composer.length() > 0) {
                return screen.state.selected_composer.c_str();
            }
            break;
        default:
            break;
        }
    }
    return page_title(screen.state.current);
}

char lower_char(char c)
{
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c + ('a' - 'A'));
    }
    return c;
}

void update_battery(UiScreen& screen)
{
    if (!screen.view.root.top_battery) {
        return;
    }
    char buf[16] = {0};
    ui_format_battery(board.getBatteryLevel(), board.isCharging(), buf, sizeof(buf));
    lv_label_set_text(screen.view.root.top_battery, buf);
    if (screen.view.root.top_signal) {
        lv_label_set_text(screen.view.root.top_signal, LV_SYMBOL_WIFI);
    }
}

void play_track(UiScreen& screen, int track_index)
{
    if (!screen.library || !screen.player) {
        return;
    }
    if (track_index < 0 || track_index >= screen.library->track_count) {
        return;
    }

    app::player_play(*screen.player, track_index);
    screen.state.last_track_index = -2;

    bool exists = false;
    for (int i = 0; i < screen.on_the_go_count; ++i) {
        if (screen.on_the_go[i] == track_index) {
            exists = true;
            break;
        }
    }
    if (!exists && screen.on_the_go_count < app::kMaxPlaylistTracks) {
        screen.on_the_go[screen.on_the_go_count++] = track_index;
    }
}

void toggle_shuffle(UiScreen& screen)
{
    if (!screen.player) {
        return;
    }
    if (screen.player->mode == app::PlaybackMode::Shuffle) {
        screen.player->mode = app::PlaybackMode::Sequential;
    } else {
        screen.player->mode = app::PlaybackMode::Shuffle;
    }
}

void toggle_repeat(UiScreen& screen)
{
    if (!screen.player) {
        return;
    }
    if (screen.player->mode == app::PlaybackMode::RepeatOne) {
        screen.player->mode = app::PlaybackMode::Sequential;
    } else {
        screen.player->mode = app::PlaybackMode::RepeatOne;
    }
}
} // namespace

void reset_items(UiScreen& screen)
{
    screen.items_count = 0;
}

ListItem* add_item(UiScreen& screen, const char* left, const char* right, UiIntentKind action, PageId next, int value,
                   int value2, const lv_image_dsc_t* icon)
{
    if (screen.items_count >= UiScreen::kMaxItems) {
        return nullptr;
    }
    ListItem& item = screen.items[screen.items_count++];
    item.left = left ? left : "";
    item.right = right ? right : "";
    item.action = action;
    item.next = next;
    item.value = value;
    item.value2 = value2;
    item.icon = icon;
    return &item;
}

ListItem* add_item(UiScreen& screen, const String& left, const String& right, UiIntentKind action, PageId next, int value,
                   int value2, const lv_image_dsc_t* icon)
{
    if (screen.items_count >= UiScreen::kMaxItems) {
        return nullptr;
    }
    ListItem& item = screen.items[screen.items_count++];
    item.left = left;
    item.right = right;
    item.action = action;
    item.next = next;
    item.value = value;
    item.value2 = value2;
    item.icon = icon;
    return &item;
}

int compare_ci(const String& a, const String& b)
{
    size_t la = a.length();
    size_t lb = b.length();
    size_t l = (la < lb) ? la : lb;
    for (size_t i = 0; i < l; ++i) {
        char ca = lower_char(a[i]);
        char cb = lower_char(b[i]);
        if (ca < cb) {
            return -1;
        }
        if (ca > cb) {
            return 1;
        }
    }
    if (la == lb) {
        return 0;
    }
    return (la < lb) ? -1 : 1;
}

void sort_string_indices(const String* arr, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (compare_ci(arr[idx[j]], arr[idx[i]]) < 0) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

void sort_album_indices(const app::Library& lib, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            const app::AlbumInfo& a = lib.albums[idx[j]];
            const app::AlbumInfo& b = lib.albums[idx[i]];
            int c = compare_ci(a.name, b.name);
            if (c == 0) {
                c = compare_ci(a.artist, b.artist);
            }
            if (c < 0) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

void sort_track_indices_by_title(const app::Library& lib, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            const app::TrackInfo& a = lib.tracks[idx[j]];
            const app::TrackInfo& b = lib.tracks[idx[i]];
            int c = compare_ci(a.title, b.title);
            if (c == 0) {
                c = compare_ci(a.artist, b.artist);
            }
            if (c < 0) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

void sort_tracks_by_added(const app::Library& lib, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            const app::TrackInfo& a = lib.tracks[idx[j]];
            const app::TrackInfo& b = lib.tracks[idx[i]];
            if (a.added_time > b.added_time) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

void sort_tracks_by_play_count(const app::Library& lib, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            const app::TrackInfo& a = lib.tracks[idx[j]];
            const app::TrackInfo& b = lib.tracks[idx[i]];
            if (a.play_count > b.play_count) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

void sort_tracks_by_last_played(const app::Library& lib, int* idx, int count)
{
    for (int i = 0; i < count - 1; ++i) {
        for (int j = i + 1; j < count; ++j) {
            const app::TrackInfo& a = lib.tracks[idx[j]];
            const app::TrackInfo& b = lib.tracks[idx[i]];
            if (a.last_played > b.last_played) {
                int tmp = idx[i];
                idx[i] = idx[j];
                idx[j] = tmp;
            }
        }
    }
}

static void populate_list_for_page(UiScreen& screen)
{
    switch (screen.state.current) {
    case PageId::MainMenu:
        screens::main_menu::populate(screen);
        break;
    case PageId::Music:
        screens::music::populate(screen);
        break;
    case PageId::Artists:
        screens::artists::populate(screen);
        break;
    case PageId::Albums:
        screens::albums::populate(screen);
        break;
    case PageId::Songs:
        screens::songs::populate(screen);
        break;
    case PageId::Genres:
        screens::genres::populate(screen);
        break;
    case PageId::Composers:
        screens::composers::populate(screen);
        break;
    case PageId::Compilations:
        screens::compilations::populate(screen);
        break;
    case PageId::Playlists:
        screens::playlists::populate(screen);
        break;
    case PageId::PlaylistDetail:
        screens::playlist_detail::populate(screen);
        break;
    case PageId::Settings:
        screens::settings::populate(screen);
        break;
    case PageId::Eq:
        screens::eq::populate(screen);
        break;
    case PageId::About:
        screens::about::populate(screen);
        break;
    default:
        reset_items(screen);
        break;
    }
}

static void build_list_page(UiScreen& screen)
{
    // Refresh strategy: list rows are rebuilt each time the page is built.
    ListPageApi api = list_api_for(screen.state.current);
    api.init_styles();
    api.apply_content(screen.view.root.content);
    screen.view.list = api.create_list(screen.view.root.content);
    api.apply_list(screen.view.list.list);

    screen.row_count = 0;
    populate_list_for_page(screen);

    for (int i = 0; i < screen.items_count; ++i) {
        if (screen.row_count >= UiScreen::kMaxItems) {
            break;
        }
        list_layout::ListRowLayout row_layout = api.create_row(screen.view.list.list);
        api.apply_row(row_layout.row);
        api.apply_left(row_layout.left_label);
        api.apply_right(row_layout.right_label);

        if (row_layout.icon) {
            bool show_icon = (screen.items[i].icon != nullptr);
            list_layout::set_row_icon_visible(row_layout, show_icon);
            if (show_icon) {
                lv_img_set_src(row_layout.icon, screen.items[i].icon);
                lv_obj_clear_flag(row_layout.icon, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(row_layout.icon, LV_OBJ_FLAG_HIDDEN);
            }
        }

        lv_label_set_text(row_layout.left_label, screen.items[i].left.c_str());

        const char* right_text = screen.items[i].right.length() > 0 ? screen.items[i].right.c_str() : nullptr;
        if (!right_text) {
            switch (screen.items[i].action) {
            case UiIntentKind::Navigate:
            case UiIntentKind::OpenAlbumsAll:
            case UiIntentKind::OpenSongsAll:
            case UiIntentKind::OpenArtist:
            case UiIntentKind::OpenAlbum:
            case UiIntentKind::OpenGenre:
            case UiIntentKind::OpenComposer:
            case UiIntentKind::OpenPlaylist:
            case UiIntentKind::OpenEq:
            case UiIntentKind::OpenAbout:
                right_text = LV_SYMBOL_RIGHT;
                break;
            default:
                break;
            }
        }
        lv_label_set_text(row_layout.right_label, right_text ? right_text : "");

        RowMeta& meta = screen.rows[screen.row_count++];
        meta.intent.kind = screen.items[i].action;
        meta.intent.next = screen.items[i].next;
        meta.intent.value = screen.items[i].value;
        meta.intent.value2 = screen.items[i].value2;
        meta.intent.item = &screen.items[i];
        meta.row = row_layout.row;
        meta.left_label = row_layout.left_label;
        meta.right_label = row_layout.right_label;
        api.attach_row(screen, meta);
    }

    lv_obj_t* first = lv_obj_get_child(screen.view.list.list, 0);
    api.focus_first(screen.group, first);
}

static void build_main_menu(UiScreen& screen)
{
    screens::main_menu::styles::init_once();
    screens::main_menu::styles::apply_content(screen.view.root.content);
    if (screen.view.root.content) {
        lv_obj_add_flag(screen.view.root.content, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    }

    screen.view.list = {};
    screen.view.menu = {};
    screen.row_count = 0;
    populate_list_for_page(screen);

    screens::main_menu::layout::MenuLayout layout =
        screens::main_menu::layout::create_menu(screen.view.root.content);
    screen.view.menu = layout;

    if (layout.arrow_left) {
        screens::main_menu::styles::apply_arrow(layout.arrow_left);
    }
    if (layout.arrow_right) {
        screens::main_menu::styles::apply_arrow(layout.arrow_right);
    }

    for (int i = 0; i < screens::main_menu::layout::kDots; ++i) {
        if (!layout.dots[i]) {
            continue;
        }
        screens::main_menu::styles::apply_dot(layout.dots[i]);
    }

    int slots = screens::main_menu::layout::kVisibleItems;
    if (slots > UiScreen::kMaxItems) {
        slots = UiScreen::kMaxItems;
    }

    for (int i = 0; i < slots; ++i) {
        if (!layout.items[i].button) {
            continue;
        }
        if (screen.row_count >= UiScreen::kMaxItems) {
            break;
        }

        screens::main_menu::styles::apply_item(layout.items[i].button);
        screens::main_menu::styles::apply_label(layout.items[i].label);

        RowMeta& meta = screen.rows[screen.row_count++];
        meta.intent = {};
        meta.row = layout.items[i].button;
        meta.left_label = layout.items[i].label;
        meta.right_label = nullptr;
        screens::main_menu::input::attach_row(screen, meta);
    }

    update_main_menu(screen);
}

void update_topbar(UiScreen& screen)
{
    if (!screen.view.root.top_left || !screen.view.root.top_title) {
        return;
    }
    if (screen.state.current == PageId::MainMenu) {
        lv_label_set_text(screen.view.root.top_left, "");
        lv_label_set_text(screen.view.root.top_title, "");
        update_battery(screen);
        return;
    }
    if (screen.state.depth > 0) {
        lv_label_set_text(screen.view.root.top_left, LV_SYMBOL_LEFT " Back");
    } else {
        lv_label_set_text(screen.view.root.top_left, "Menu");
    }

    lv_label_set_text(screen.view.root.top_title, current_page_title(screen));
    update_battery(screen);
}

void update_now_playing(UiScreen& screen)
{
    screens::now_playing::update(screen);
}

void update_main_menu(UiScreen& screen)
{
    auto& menu = screen.view.menu;
    if (!menu.wrap) {
        return;
    }

    int total = screen.items_count;
    if (total <= 0) {
        for (int i = 0; i < screens::main_menu::layout::kVisibleItems; ++i) {
            if (menu.items[i].button) {
                lv_obj_add_flag(menu.items[i].button, LV_OBJ_FLAG_HIDDEN);
            }
        }
        return;
    }

    if (screen.state.menu_index < 0) {
        screen.state.menu_index = 0;
    } else if (screen.state.menu_index >= total) {
        screen.state.menu_index = total - 1;
    }

    int visible = screens::main_menu::layout::kVisibleItems;
    if (visible > total) {
        visible = total;
    }

    constexpr float kSideScale = 0.6f;
    constexpr lv_coord_t kIconOffsetY = 0;
    const lv_coord_t slot_width = menu.icon_size + menu.icon_gap;

    for (int i = 0; i < screens::main_menu::layout::kVisibleItems; ++i) {
        if (!menu.items[i].button) {
            continue;
        }
        if (i >= visible) {
            lv_obj_add_flag(menu.items[i].button, LV_OBJ_FLAG_HIDDEN);
            continue;
        }

        int item_idx = screen.state.menu_index + (i - 1);
        if (total > 0) {
            item_idx %= total;
            if (item_idx < 0) {
                item_idx += total;
            }
        }

        lv_obj_clear_flag(menu.items[i].button, LV_OBJ_FLAG_HIDDEN);
        screens::main_menu::layout::set_item(menu.items[i], screen.items[item_idx].left.c_str(),
                                             screen.items[item_idx].icon);

        float scale = (i == 1 || visible == 1) ? 1.0f : kSideScale;
        lv_coord_t btn_w = menu.icon_size;
        lv_coord_t btn_h = menu.item_height;

        lv_coord_t slot_left = menu.row_left + i * slot_width;
        lv_coord_t slot_center = slot_left + (menu.icon_size / 2);
        lv_coord_t btn_left = slot_center - (btn_w / 2);
        lv_coord_t btn_top = menu.icon_top;

        lv_obj_set_size(menu.items[i].button, btn_w, btn_h);
        lv_obj_align(menu.items[i].button, LV_ALIGN_TOP_LEFT, btn_left, btn_top);

        if (menu.items[i].icon) {
            lv_coord_t icon_w = static_cast<lv_coord_t>(menu.icon_size * scale);
            if (icon_w < 1) {
                icon_w = 1;
            }
            lv_coord_t icon_y = (menu.icon_size - icon_w) / 2 + kIconOffsetY;
            lv_obj_set_size(menu.items[i].icon, icon_w, icon_w);
            lv_obj_align(menu.items[i].icon, LV_ALIGN_TOP_MID, 0, icon_y);
            const lv_image_dsc_t* icon = screen.items[item_idx].icon;
            if (icon && icon->header.w > 0) {
                uint32_t zoom =
                    (static_cast<uint32_t>(icon_w) * 256U) / static_cast<uint32_t>(icon->header.w);
                if (zoom == 0) {
                    zoom = 256;
                }
                lv_img_set_zoom(menu.items[i].icon, zoom);
            }
            lv_obj_set_style_img_opa(menu.items[i].icon, (scale >= 0.99f) ? LV_OPA_COVER : LV_OPA_70, LV_PART_MAIN);
        }
        if (menu.items[i].label) {
            lv_obj_set_width(menu.items[i].label, menu.icon_size + menu.icon_gap);
            lv_coord_t label_y = menu.label_top - menu.icon_top;
            lv_obj_align(menu.items[i].label, LV_ALIGN_TOP_MID, 0, label_y);
            lv_obj_set_style_text_opa(menu.items[i].label, (scale >= 0.99f) ? LV_OPA_COVER : LV_OPA_70, LV_PART_MAIN);
        }

        RowMeta& meta = screen.rows[i];
        meta.intent.kind = screen.items[item_idx].action;
        meta.intent.next = screen.items[item_idx].next;
        meta.intent.value = screen.items[item_idx].value;
        meta.intent.value2 = screen.items[item_idx].value2;
        meta.intent.item = &screen.items[item_idx];
    }

    int active_dot = (total > 0) ? (screen.state.menu_index % screens::main_menu::layout::kDots) : 0;
    for (int i = 0; i < screens::main_menu::layout::kDots; ++i) {
        if (!menu.dots[i]) {
            continue;
        }
        lv_obj_clear_flag(menu.dots[i], LV_OBJ_FLAG_HIDDEN);
        screens::main_menu::layout::set_dot_active(menu.dots[i], i == active_dot);
    }

    int focus_slot = (visible >= 2) ? 1 : 0;
    if (focus_slot >= visible) {
        focus_slot = 0;
    }
    if (menu.items[focus_slot].button && screen.group) {
        lv_group_focus_obj(menu.items[focus_slot].button);
    }

}

NavCommand handle_intent(UiScreen& screen, const UiIntent& intent)
{
    NavCommand cmd{};

    switch (intent.kind) {
    case UiIntentKind::Navigate:
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = intent.next;
        break;
    case UiIntentKind::NavigateBack:
        cmd.type = NavCommand::Type::Back;
        break;
    case UiIntentKind::OpenAlbumsAll:
        screen.state.album_filter = AlbumFilter::All;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Albums;
        break;
    case UiIntentKind::OpenSongsAll:
        screen.state.song_context = SongContext::All;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Songs;
        break;
    case UiIntentKind::OpenArtist:
        if (intent.item) {
            screen.state.selected_artist = intent.item->left;
        }
        screen.state.album_filter = AlbumFilter::Artist;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Albums;
        break;
    case UiIntentKind::OpenAlbum:
        if (intent.item) {
            screen.state.selected_album = intent.item->left;
            screen.state.selected_album_artist = intent.item->right;
        }
        screen.state.song_context = SongContext::Album;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Songs;
        break;
    case UiIntentKind::OpenGenre:
        if (intent.item) {
            screen.state.selected_genre = intent.item->left;
        }
        screen.state.song_context = SongContext::Genre;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Songs;
        break;
    case UiIntentKind::OpenComposer:
        if (intent.item) {
            screen.state.selected_composer = intent.item->left;
        }
        screen.state.song_context = SongContext::Composer;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Songs;
        break;
    case UiIntentKind::OpenPlaylist:
        screen.state.current_playlist = intent.value;
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::PlaylistDetail;
        break;
    case UiIntentKind::PlayTrack:
        play_track(screen, intent.value);
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::NowPlaying;
        break;
    case UiIntentKind::ToggleShuffle:
        toggle_shuffle(screen);
        cmd.type = NavCommand::Type::Rebuild;
        break;
    case UiIntentKind::ToggleRepeat:
        toggle_repeat(screen);
        cmd.type = NavCommand::Type::Rebuild;
        break;
    case UiIntentKind::OpenEq:
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::Eq;
        break;
    case UiIntentKind::OpenAbout:
        cmd.type = NavCommand::Type::NavigateTo;
        cmd.target = PageId::About;
        break;
    case UiIntentKind::PrevTrack:
        if (screen.player) {
            app::player_prev(*screen.player);
            screen.state.last_track_index = -2;
        }
        break;
    case UiIntentKind::NextTrack:
        if (screen.player) {
            app::player_next(*screen.player);
            screen.state.last_track_index = -2;
        }
        break;
    case UiIntentKind::TogglePause:
        if (screen.player) {
            app::player_toggle_pause(*screen.player);
        }
        break;
    default:
        break;
    }

    return cmd;
}

void build_page(UiScreen& screen)
{
    shell_styles::init_once();
    screen.view.root = shell_layout::create_root();

    lv_obj_t* active = lv_screen_active();
    if (active) {
        lv_obj_set_style_bg_color(active, lv_color_hex(0x0b0b0b), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(active, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_width(active, 0, LV_PART_MAIN);
        lv_obj_set_style_outline_width(active, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(active, 0, LV_PART_MAIN);
    }

    shell_styles::apply_root(screen.view.root.root);
    shell_styles::apply_topbar(screen.view.root.topbar);
    shell_styles::apply_topbar_label(screen.view.root.top_left);
    shell_styles::apply_topbar_title(screen.view.root.top_title);
    shell_styles::apply_topbar_status(screen.view.root.top_status);
    shell_styles::apply_topbar_label(screen.view.root.top_signal);
    shell_styles::apply_topbar_label(screen.view.root.top_battery);

    if (!screen.group) {
        screen.group = lv_group_create();
    }
    set_default_group(screen.group);

    update_topbar(screen);

    if (page_needs_library(screen.state.current)) {
        ensure_library_scanned(screen);
    }

    if (screen.state.current == PageId::NowPlaying) {
        screens::now_playing::build(screen);
    } else if (screen.state.current == PageId::MainMenu) {
        build_main_menu(screen);
    } else {
        build_list_page(screen);
    }

    update_now_playing(screen);
}

} // namespace lofi::ui::components
