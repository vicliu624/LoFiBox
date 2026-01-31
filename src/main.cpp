#include <Arduino.h>
#include <SD.h>

#include "app/library.h"
#include "app/player.h"
#include "board/BoardBase.h"
#include "ui/LV_Helper.h"
#include "ui/assets/assets.h"
#include "ui/fonts/fonts.h"
#include "ui/lofibox/lofibox_ui.h"

namespace
{
app::Library s_library{};
app::PlayerState s_player{};
lv_obj_t* s_boot_root = nullptr;
lv_obj_t* s_boot_label = nullptr;

void boot_tick()
{
    lvHelperTick();
    lv_timer_handler();
    delay(1);
}

void show_boot_screen()
{
    init_font_fallbacks();

    s_boot_root = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(s_boot_root);
    lv_obj_set_size(s_boot_root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_boot_root, lv_color_hex(0x0b0b0b), 0);
    lv_obj_set_style_bg_opa(s_boot_root, LV_OPA_COVER, 0);

    lv_obj_t* img = lv_img_create(s_boot_root);
    lv_img_set_src(img, &logo);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, -16);

    s_boot_label = lv_label_create(s_boot_root);
    lv_label_set_text(s_boot_label, "Loading...");
    lv_obj_set_style_text_color(s_boot_label, lv_color_hex(0xf2f2f2), 0);
    lv_obj_set_style_text_font(s_boot_label, font_noto_sc_16(), 0);
    lv_obj_align_to(s_boot_label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);

    boot_tick();
}

void hide_boot_screen()
{
    if (s_boot_root) {
        lv_obj_del(s_boot_root);
        s_boot_root = nullptr;
        s_boot_label = nullptr;
    }
}
}

void setup()
{
    Serial.begin(115200);
    delay(50);

    board.begin();
    beginLvglHelper();
    randomSeed(micros());

    app::library_reset(s_library);
    show_boot_screen();
    const uint32_t boot_start = millis();
    app::library_scan(s_library, SD, "/music", 8, app::kMaxTracks, true, boot_tick);
    uint32_t elapsed = millis() - boot_start;
    while (elapsed < 3000) {
        boot_tick();
        elapsed = millis() - boot_start;
    }
    hide_boot_screen();

    app::player_init(s_player, s_library);
    lofi::ui::init(&s_library, &s_player);

}

void loop()
{
    app::player_loop(s_player);
    lofi::ui::tick();
    lvHelperTick();
    lv_timer_handler();
    delay(2);
}
