#include "app/eq_dsp.h"
#include "app/player.h"

void audio_process_extern(int16_t* buff, uint16_t len, bool* continueI2S)
{
    if (continueI2S) {
        *continueI2S = true;
    }
    if (!buff || len == 0) {
        return;
    }

    if (app::player_bits_per_sample() != 16) {
        return;
    }

    const uint32_t sr = app::player_sample_rate();
    const int channels = static_cast<int>(app::player_channels());
    app::eq::process_block(buff, len, channels, sr);
}
