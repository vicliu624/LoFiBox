#include <Arduino.h>
#include <SD.h>

#include "app/library.h"
#include "app/player.h"
#include "board/BoardBase.h"
#include "ui/LV_Helper.h"
#include "ui/lofibox/lofibox_ui.h"

namespace
{
app::Library s_library{};
app::PlayerState s_player{};
}

void setup()
{
    Serial.begin(115200);
    delay(50);

    board.begin();
    beginLvglHelper();
    randomSeed(micros());

    app::library_reset(s_library);
    app::player_init(s_player, s_library);
    lofi::ui::init(&s_library, &s_player);

}

void loop()
{
    app::player_loop(s_player);
    lofi::ui::tick();
    lv_timer_handler();
    delay(2);
}
