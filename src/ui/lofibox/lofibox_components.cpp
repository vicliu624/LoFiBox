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

namespace lofi::ui::components
{
namespace
{
namespace shell_layout = screens::common::layout;
namespace shell_styles = screens::common::styles;
namespace list_layout = screens::list_page::layout;

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
    case PageId::MainMenu:
        return LIST_API(screens::main_menu);
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
        return LIST_API(screens::main_menu);
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

void update_topbar(UiScreen& screen)
{
    if (!screen.view.root.top_left || !screen.view.root.top_title) {
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

    if (screen.state.current == PageId::NowPlaying) {
        screens::now_playing::build(screen);
    } else {
        build_list_page(screen);
    }

    update_now_playing(screen);
}

} // namespace lofi::ui::components
