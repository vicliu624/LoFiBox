#include "ui/lofibox/lofibox_components.h"

#include "board/BoardBase.h"
#include "ui/screens/common/shell_layout.h"
#include "ui/screens/common/shell_styles.h"
#include "ui/screens/eq/eq_components.h"
#include "ui/screens/list_page/list_page_build.h"
#include "ui/screens/main_menu/main_menu_components.h"
#include "ui/screens/main_menu/main_menu_input.h"
#include "ui/screens/main_menu/main_menu_layout.h"
#include "ui/screens/main_menu/main_menu_styles.h"
#include "ui/screens/now_playing/now_playing_components.h"
#include "ui/ui_common.h"
#include "ui/common/text_utils.h"
#include "app/library.h"

namespace lofi::ui::components
{
namespace
{
namespace shell_layout = screens::common::layout;
namespace shell_styles = screens::common::styles;
using ListPageApi = screens::list_page::BuildApi;

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

    const app::TrackInfo& track = screen.library->tracks[track_index];
    Serial.printf("[UI] play_track idx=%d title=\"%s\" artist=\"%s\" album=\"%s\"\n",
                  track_index,
                  track.title ? track.title : "",
                  track.artist ? track.artist : "",
                  track.album ? track.album : "");
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
    screens::main_menu::populate(screen);

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
        if (screen.view.root.top_title_fade_left) {
            lv_obj_add_flag(screen.view.root.top_title_fade_left, LV_OBJ_FLAG_HIDDEN);
        }
        if (screen.view.root.top_title_fade_right) {
            lv_obj_add_flag(screen.view.root.top_title_fade_right, LV_OBJ_FLAG_HIDDEN);
        }
        update_battery(screen);
        return;
    }
    if (screen.state.current == PageId::NowPlaying && screen.player && screen.library &&
        screen.player->current_index >= 0 && screen.player->current_index < screen.library->track_count) {
        const app::TrackInfo& track = screen.library->tracks[screen.player->current_index];
        String safe_title = text::single_line((track.title && track.title[0]) ? track.title : "Now Playing");
        String safe_album = text::single_line(track.album ? track.album : "");
        String safe_genre = text::single_line(track.genre ? track.genre : "");
        lv_label_set_long_mode(screen.view.root.top_left, LV_LABEL_LONG_CLIP);
        lv_label_set_long_mode(screen.view.root.top_title, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_style_text_align(screen.view.root.top_left, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
        lv_obj_set_style_text_align(screen.view.root.top_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(screen.view.root.top_left, 0, LV_PART_MAIN);
        lv_obj_set_style_text_line_space(screen.view.root.top_title, 0, LV_PART_MAIN);

        lv_coord_t w = lv_display_get_horizontal_resolution(nullptr);
        if (w <= 0) {
            w = 480;
        }
        const lv_font_t* font_left = lv_obj_get_style_text_font(screen.view.root.top_left, LV_PART_MAIN);
        if (font_left) {
            lv_coord_t char_w = lv_font_get_glyph_width(font_left, 'W', 0);
            if (char_w > 0) {
                lv_obj_set_width(screen.view.root.top_left, char_w * 2);
            } else {
                lv_obj_set_width(screen.view.root.top_left, (w * 10) / 100);
            }
        } else {
            lv_obj_set_width(screen.view.root.top_left, (w * 10) / 100);
        }
        lv_coord_t title_w = (w * 45) / 100;
        const lv_font_t* font_title = lv_obj_get_style_text_font(screen.view.root.top_title, LV_PART_MAIN);
        if (font_title) {
            lv_coord_t char_w = lv_font_get_glyph_width(font_title, 'W', 0);
            if (char_w > 0) {
                title_w = char_w * 18;
            } else {
                title_w = (w * 45) / 100;
            }
        } else {
            title_w = (w * 45) / 100;
        }
        lv_obj_set_height(screen.view.root.top_left, LV_SIZE_CONTENT);
        lv_obj_set_height(screen.view.root.top_title, LV_SIZE_CONTENT);

        lv_label_set_text(screen.view.root.top_left, LV_SYMBOL_LEFT);

        String center;
        if (safe_album.length() > 0 && safe_genre.length() > 0) {
            center = safe_album + " / " + safe_genre;
        } else if (safe_album.length() > 0) {
            center = safe_album;
        } else if (safe_genre.length() > 0) {
            center = safe_genre;
        } else {
            center = "";
        }
        lv_label_set_text(screen.view.root.top_title, center.c_str());
        if (screen.view.root.top_title_wrap) {
            lv_obj_set_width(screen.view.root.top_title_wrap, title_w);
            lv_obj_set_height(screen.view.root.top_title_wrap, LV_SIZE_CONTENT);
            lv_obj_align(screen.view.root.top_title_wrap, LV_ALIGN_CENTER, 0, 0);
        }
        lv_obj_set_width(screen.view.root.top_title, title_w);
        lv_obj_align(screen.view.root.top_title, LV_ALIGN_LEFT_MID, 0, 0);

        lv_obj_set_style_text_color(screen.view.root.top_left, lv_color_hex(0xf2f2f2), LV_PART_MAIN);
        lv_obj_set_style_text_color(screen.view.root.top_title, lv_color_hex(0x8f949a), LV_PART_MAIN);

        if (screen.view.root.top_title_fade_left && screen.view.root.top_title_fade_right) {
            bool show_fade = true;
            const lv_font_t* font = lv_obj_get_style_text_font(screen.view.root.top_title, LV_PART_MAIN);
            if (font) {
                lv_point_t text_size{};
                lv_coord_t letter_space = lv_obj_get_style_text_letter_space(screen.view.root.top_title, LV_PART_MAIN);
                lv_coord_t line_space = lv_obj_get_style_text_line_space(screen.view.root.top_title, LV_PART_MAIN);
                lv_txt_get_size(&text_size, center.c_str(), font, letter_space, line_space, LV_COORD_MAX,
                                LV_TEXT_FLAG_NONE);
                if (text_size.x <= title_w - 2) {
                    show_fade = false;
                }
            }

            lv_coord_t title_w_actual = lv_obj_get_width(screen.view.root.top_title);
            if (title_w_actual > 0) {
                title_w = title_w_actual;
            }
            lv_coord_t bar_h = lv_obj_get_height(screen.view.root.top_title_wrap
                                                     ? screen.view.root.top_title_wrap
                                                     : screen.view.root.topbar);
            lv_coord_t fade_w = title_w / 12;
            if (fade_w < 6) {
                fade_w = 6;
            }
            if (fade_w > 10) {
                fade_w = 10;
            }
            if (show_fade) {
                lv_obj_clear_flag(screen.view.root.top_title_fade_left, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(screen.view.root.top_title_fade_right, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_size(screen.view.root.top_title_fade_left, fade_w, bar_h);
                lv_obj_align(screen.view.root.top_title_fade_left, LV_ALIGN_LEFT_MID, 0, 0);
                lv_obj_set_size(screen.view.root.top_title_fade_right, fade_w, bar_h);
                lv_obj_align(screen.view.root.top_title_fade_right, LV_ALIGN_RIGHT_MID, 0, 0);
                lv_obj_move_foreground(screen.view.root.top_title_fade_left);
                lv_obj_move_foreground(screen.view.root.top_title_fade_right);
            } else {
                lv_obj_add_flag(screen.view.root.top_title_fade_left, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(screen.view.root.top_title_fade_right, LV_OBJ_FLAG_HIDDEN);
            }
        }

        update_battery(screen);
        return;
    }
    if (screen.state.depth > 0) {
        lv_label_set_text(screen.view.root.top_left, LV_SYMBOL_LEFT " Back");
    } else {
        lv_label_set_text(screen.view.root.top_left, "Menu");
    }

    if (screen.view.root.top_title_fade_left) {
        lv_obj_add_flag(screen.view.root.top_title_fade_left, LV_OBJ_FLAG_HIDDEN);
    }
    if (screen.view.root.top_title_fade_right) {
        lv_obj_add_flag(screen.view.root.top_title_fade_right, LV_OBJ_FLAG_HIDDEN);
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
    if (screen.view.root.top_title_wrap) {
        lv_obj_set_style_bg_opa(screen.view.root.top_title_wrap, LV_OPA_0, LV_PART_MAIN);
        lv_obj_set_style_border_width(screen.view.root.top_title_wrap, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(screen.view.root.top_title_wrap, 0, LV_PART_MAIN);
    }
    shell_styles::apply_topbar_title(screen.view.root.top_title);
    shell_styles::apply_topbar_status(screen.view.root.top_status);
    shell_styles::apply_topbar_label(screen.view.root.top_signal);
    shell_styles::apply_topbar_label(screen.view.root.top_battery);
    shell_styles::apply_topbar_fade_left(screen.view.root.top_title_fade_left);
    shell_styles::apply_topbar_fade_right(screen.view.root.top_title_fade_right);

    if (!screen.group) {
        screen.group = lv_group_create();
    }
    set_default_group(screen.group);

    update_topbar(screen);

    if (screen.state.current == PageId::NowPlaying) {
        screens::now_playing::build(screen);
    } else if (screen.state.current == PageId::MainMenu) {
        build_main_menu(screen);
    } else if (screen.state.current == PageId::Eq) {
        screens::eq::build(screen);
    } else {
        screens::list_page::populate_list(screen);
        ListPageApi api = screens::list_page::api_for(screen.state.current);
        screens::list_page::build_list_page(screen, api);
    }

    update_now_playing(screen);
}

} // namespace lofi::ui::components
