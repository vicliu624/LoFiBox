#include "ui/lofibox/lofibox_ui.h"

#include "ui/lofibox/lofibox_components.h"
#include "ui/lofibox/lofibox_ui_internal.h"
#include "ui/fonts/fonts.h"

namespace lofi::ui
{
namespace
{
UiScreen s_screen;

void timer_delete(lv_timer_t* timer)
{
#if defined(LVGL_VERSION_MAJOR) && (LVGL_VERSION_MAJOR >= 9)
    lv_timer_delete(timer);
#else
    lv_timer_del(timer);
#endif
}

void on_root_delete(lv_event_t* e)
{
    auto* screen = static_cast<UiScreen*>(lv_event_get_user_data(e));
    if (!screen) {
        return;
    }

    screen->alive = false;
    screen->has_pending_intent = false;
    screen->timers.clear_all();
    screen->view = {};

    if (screen->group) {
        lv_group_del(screen->group);
        screen->group = nullptr;
    }
}

void attach_delete_hook(UiScreen& screen)
{
    if (!screen.view.root.root) {
        return;
    }
    lv_obj_add_event_cb(screen.view.root.root, on_root_delete, LV_EVENT_DELETE, &screen);
}

void battery_timer_cb(lv_timer_t* timer)
{
    auto* screen = static_cast<UiScreen*>(timer ? lv_timer_get_user_data(timer) : nullptr);
    if (!screen_alive(screen)) {
        return;
    }
    components::update_topbar(*screen);
}

void now_playing_timer_cb(lv_timer_t* timer)
{
    auto* screen = static_cast<UiScreen*>(timer ? lv_timer_get_user_data(timer) : nullptr);
    if (!screen_alive(screen)) {
        return;
    }
    components::update_now_playing(*screen);
}

void start_timers(UiScreen& screen)
{
    screen.timers.reset();
    screen.timers.create(TimerDomain::ScreenGeneral, battery_timer_cb, 60000, &screen);
    screen.timers.create(TimerDomain::NowPlaying, now_playing_timer_cb, 500, &screen);
}

void destroy_view(UiScreen& screen)
{
    if (screen.view.root.root) {
        lv_obj_del(screen.view.root.root);
    }
}

void build_view(UiScreen& screen)
{
    screen.alive = true;
    components::build_page(screen);
    attach_delete_hook(screen);
    start_timers(screen);
}

void navigate_to(UiScreen& screen, PageId id, bool push)
{
    if (push && screen.state.current != PageId::None &&
        screen.state.depth < static_cast<int>(sizeof(screen.state.stack) / sizeof(screen.state.stack[0]))) {
        screen.state.stack[screen.state.depth++] = screen.state.current;
    }

    screen.state.current = id;
    destroy_view(screen);
    build_view(screen);
}

void navigate_back(UiScreen& screen)
{
    if (screen.state.depth <= 0) {
        return;
    }
    PageId prev = screen.state.stack[--screen.state.depth];
    screen.state.current = prev;
    destroy_view(screen);
    build_view(screen);
}

void rebuild_current(UiScreen& screen)
{
    destroy_view(screen);
    build_view(screen);
}
} // namespace

void TimerBag::reset()
{
    clear_all();
}

lv_timer_t* TimerBag::create(TimerDomain domain, lv_timer_cb_t cb, uint32_t period, void* user_data)
{
    if (count >= kMaxTimers) {
        return nullptr;
    }

    lv_timer_t* timer = lv_timer_create(cb, period, user_data);
    if (!timer) {
        return nullptr;
    }

    entries[count].timer = timer;
    entries[count].domain = domain;
    ++count;
    return timer;
}

void TimerBag::clear_domain(TimerDomain domain)
{
    for (int i = 0; i < count;) {
        if (entries[i].timer && entries[i].domain == domain) {
            timer_delete(entries[i].timer);
            for (int j = i + 1; j < count; ++j) {
                entries[j - 1] = entries[j];
            }
            --count;
            continue;
        }
        ++i;
    }
}

void TimerBag::clear_all()
{
    for (int i = 0; i < count; ++i) {
        if (entries[i].timer) {
            timer_delete(entries[i].timer);
        }
    }
    count = 0;
}

void request_intent(UiScreen* screen, const UiIntent& intent)
{
    if (!screen_alive(screen)) {
        return;
    }
    screen->pending_intent = intent;
    screen->has_pending_intent = true;
}

bool screen_alive(const UiScreen* screen)
{
    return screen && screen->alive;
}

void init(app::Library* library, app::PlayerState* player)
{
    init_font_fallbacks();
    s_screen.alive = false;
    s_screen.has_pending_intent = false;
    s_screen.pending_intent = {};
    s_screen.timers.clear_all();
    s_screen.view = {};
    s_screen.items_count = 0;
    s_screen.row_count = 0;
    s_screen.on_the_go_count = 0;
    s_screen.playlist_count = 0;
    if (s_screen.group) {
        lv_group_del(s_screen.group);
        s_screen.group = nullptr;
    }
    s_screen.library = library;
    s_screen.player = player;
    s_screen.state = {};
    s_screen.state.current = PageId::MainMenu;
    s_screen.state.song_context = SongContext::All;
    s_screen.state.album_filter = AlbumFilter::All;
    s_screen.state.current_playlist = -1;
    s_screen.state.last_track_index = -2;

    build_view(s_screen);
}

void tick()
{
    if (!screen_alive(&s_screen)) {
        return;
    }
    if (!s_screen.has_pending_intent) {
        return;
    }

    UiIntent intent = s_screen.pending_intent;
    s_screen.has_pending_intent = false;

    components::NavCommand cmd = components::handle_intent(s_screen, intent);
    switch (cmd.type) {
    case components::NavCommand::Type::NavigateTo:
        navigate_to(s_screen, cmd.target, true);
        break;
    case components::NavCommand::Type::Back:
        navigate_back(s_screen);
        break;
    case components::NavCommand::Type::Rebuild:
        rebuild_current(s_screen);
        break;
    default:
        break;
    }
}

} // namespace lofi::ui
