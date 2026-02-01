#include "app/eq_dsp.h"

#include <math.h>
#include <string.h>

namespace app::eq
{
namespace
{
constexpr float kDefaultQ = 1.2f;
constexpr float kBandFreqs[kBandCount] = {60.0f, 200.0f, 400.0f, 1000.0f, 2500.0f, 8000.0f};

struct Biquad
{
    int32_t b0 = 0;
    int32_t b1 = 0;
    int32_t b2 = 0;
    int32_t a1 = 0;
    int32_t a2 = 0;
    int64_t s1_l = 0;
    int64_t s2_l = 0;
    int64_t s1_r = 0;
    int64_t s2_r = 0;
};

struct State
{
    Settings settings{};
    Settings target{};
    uint32_t sample_rate = 44100;
    bool dirty = true;
    int32_t preamp_q30 = 0;
    Biquad bands[kBandCount];
};

State s_state;

int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi)
{
    if (v < lo) {
        return lo;
    }
    if (v > hi) {
        return hi;
    }
    return v;
}

int32_t clamp_q30(int64_t v)
{
    if (v > 2147483647LL) {
        return 2147483647;
    }
    if (v < -2147483648LL) {
        return -2147483648LL;
    }
    return static_cast<int32_t>(v);
}

int16_t clamp_i16(int32_t v)
{
    if (v > 32767) {
        return 32767;
    }
    if (v < -32768) {
        return -32768;
    }
    return static_cast<int16_t>(v);
}

int32_t float_to_q30(float v)
{
    const float scaled = v * 1073741824.0f;
    if (scaled >= 2147483647.0f) {
        return 2147483647;
    }
    if (scaled <= -2147483648.0f) {
        return -2147483648LL;
    }
    return static_cast<int32_t>(lrintf(scaled));
}

int32_t mul_q30(int32_t a, int32_t b)
{
    return static_cast<int32_t>((static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 30);
}

void recalc_band(int band, float gain_db, float sample_rate)
{
    if (band < 0 || band >= kBandCount) {
        return;
    }
    if (sample_rate <= 0.0f) {
        return;
    }

    const float A = powf(10.0f, gain_db / 40.0f);
    constexpr float kPi = 3.14159265358979323846f;
    const float w0 = 2.0f * kPi * (kBandFreqs[band] / sample_rate);
    const float cosw = cosf(w0);
    const float sinw = sinf(w0);
    const float alpha = sinw / (2.0f * kDefaultQ);

    const float b0 = 1.0f + alpha * A;
    const float b1 = -2.0f * cosw;
    const float b2 = 1.0f - alpha * A;
    const float a0 = 1.0f + alpha / A;
    const float a1 = -2.0f * cosw;
    const float a2 = 1.0f - alpha / A;

    const float inv_a0 = (a0 != 0.0f) ? (1.0f / a0) : 1.0f;

    s_state.bands[band].b0 = float_to_q30(b0 * inv_a0);
    s_state.bands[band].b1 = float_to_q30(b1 * inv_a0);
    s_state.bands[band].b2 = float_to_q30(b2 * inv_a0);
    s_state.bands[band].a1 = float_to_q30(a1 * inv_a0);
    s_state.bands[band].a2 = float_to_q30(a2 * inv_a0);
}

void recalc_all()
{
    const float sr = static_cast<float>(s_state.sample_rate);
    for (int i = 0; i < kBandCount; ++i) {
        recalc_band(i, static_cast<float>(s_state.settings.band_db[i]), sr);
    }

    const float preamp_gain = powf(10.0f, static_cast<float>(s_state.settings.preamp_db) / 20.0f);
    s_state.preamp_q30 = float_to_q30(preamp_gain);
}

void apply_target()
{
    s_state.settings = s_state.target;
    s_state.dirty = true;
}

void update_enabled_from_target()
{
    bool any = (s_state.target.preamp_db != 0);
    if (!any) {
        for (int i = 0; i < kBandCount; ++i) {
            if (s_state.target.band_db[i] != 0) {
                any = true;
                break;
            }
        }
    }
    s_state.target.enabled = any;
}
} // namespace

void init()
{
    memset(&s_state, 0, sizeof(s_state));
    s_state.settings.enabled = false;
    s_state.settings.preamp_db = 0;
    s_state.target = s_state.settings;
    s_state.sample_rate = 44100;
    s_state.dirty = true;
}

Settings get_settings()
{
    return s_state.settings;
}

int8_t get_band(int band)
{
    if (band < 0 || band >= kBandCount) {
        return 0;
    }
    return s_state.target.band_db[band];
}

void set_band(int band, int8_t db)
{
    if (band < 0 || band >= kBandCount) {
        return;
    }
    s_state.target.band_db[band] = static_cast<int8_t>(clamp_i32(db, -12, 12));
    update_enabled_from_target();
    apply_target();
}

int8_t get_preamp()
{
    return s_state.target.preamp_db;
}

void set_preamp(int8_t db)
{
    s_state.target.preamp_db = static_cast<int8_t>(clamp_i32(db, -12, 0));
    update_enabled_from_target();
    apply_target();
}

bool is_enabled()
{
    return s_state.settings.enabled;
}

void set_enabled(bool enabled)
{
    s_state.target.enabled = enabled;
    apply_target();
}

void set_sample_rate(uint32_t sample_rate)
{
    if (sample_rate == 0) {
        return;
    }
    if (s_state.sample_rate == sample_rate) {
        return;
    }
    s_state.sample_rate = sample_rate;
    s_state.dirty = true;
}

void process_block(int16_t* buffer, uint16_t frames, int channels, uint32_t sample_rate)
{
    if (!buffer || frames == 0) {
        return;
    }
    if (channels < 1) {
        channels = 1;
    }
    if (channels > 2) {
        channels = 2;
    }

    if (sample_rate != 0) {
        set_sample_rate(sample_rate);
    }

    if (!s_state.settings.enabled) {
        return;
    }

    if (s_state.dirty) {
        recalc_all();
        s_state.dirty = false;
    }

    const int32_t preamp = s_state.preamp_q30;

    if (channels == 1) {
        for (uint16_t i = 0; i < frames; ++i) {
            int32_t x = static_cast<int32_t>(buffer[i]) << 15;
            x = mul_q30(x, preamp);
            for (int b = 0; b < kBandCount; ++b) {
                Biquad& f = s_state.bands[b];
                int64_t y = (static_cast<int64_t>(f.b0) * x) >> 30;
                y += f.s1_l;
                int64_t s1 = static_cast<int64_t>(f.b1) * x;
                s1 >>= 30;
                s1 -= (static_cast<int64_t>(f.a1) * y) >> 30;
                s1 += f.s2_l;
                int64_t s2 = (static_cast<int64_t>(f.b2) * x) >> 30;
                s2 -= (static_cast<int64_t>(f.a2) * y) >> 30;
                f.s1_l = s1;
                f.s2_l = s2;
                x = clamp_q30(y);
            }
            int32_t out = x >> 15;
            buffer[i] = clamp_i16(out);
        }
        return;
    }

    for (uint16_t i = 0; i < frames; ++i) {
        int32_t xl = static_cast<int32_t>(buffer[i * 2]) << 15;
        int32_t xr = static_cast<int32_t>(buffer[i * 2 + 1]) << 15;
        xl = mul_q30(xl, preamp);
        xr = mul_q30(xr, preamp);

        for (int b = 0; b < kBandCount; ++b) {
            Biquad& f = s_state.bands[b];
            int64_t yl = (static_cast<int64_t>(f.b0) * xl) >> 30;
            yl += f.s1_l;
            int64_t s1l = static_cast<int64_t>(f.b1) * xl;
            s1l >>= 30;
            s1l -= (static_cast<int64_t>(f.a1) * yl) >> 30;
            s1l += f.s2_l;
            int64_t s2l = (static_cast<int64_t>(f.b2) * xl) >> 30;
            s2l -= (static_cast<int64_t>(f.a2) * yl) >> 30;
            f.s1_l = s1l;
            f.s2_l = s2l;
            xl = clamp_q30(yl);

            int64_t yr = (static_cast<int64_t>(f.b0) * xr) >> 30;
            yr += f.s1_r;
            int64_t s1r = static_cast<int64_t>(f.b1) * xr;
            s1r >>= 30;
            s1r -= (static_cast<int64_t>(f.a1) * yr) >> 30;
            s1r += f.s2_r;
            int64_t s2r = (static_cast<int64_t>(f.b2) * xr) >> 30;
            s2r -= (static_cast<int64_t>(f.a2) * yr) >> 30;
            f.s1_r = s1r;
            f.s2_r = s2r;
            xr = clamp_q30(yr);
        }

        buffer[i * 2] = clamp_i16(xl >> 15);
        buffer[i * 2 + 1] = clamp_i16(xr >> 15);
    }
}

} // namespace app::eq
