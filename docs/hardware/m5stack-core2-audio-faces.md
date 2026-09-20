# M5Stack Core2 + Audio Module + Faces Gamepad3

This target treats the following stack as a dedicated LoFiBox hardware backend:

- M5Stack Core2 v1.3 (`m5stack-core2`)
- M5Stack Audio Module M144, installed in **Configuration A** for Basic/Core2
- Faces Bottom3 with Faces Gamepad3

It uses `M5Unified`; the deprecated `M5Core2` library is not used.

> Before powering the stack, set Bottom3's physical **LED IO** selector to
> **`1 / G25`**. The LED data input is selected in hardware; software cannot
> switch it from the legacy G15 route. `Bottom3 Music LEDs` in Settings
> controls only the ten Bottom3 side LEDs (two parallel five-pixel bars), not
> the Audio Module or Gamepad-panel LEDs.

## Build

```bash
pio run -e core2_audio_faces
pio run -t upload -e core2_audio_faces
pio device monitor -e core2_audio_faces
```

The Core2 environment defines `BOARD_HAS_PSRAM`. LVGL's full screenshot frame
buffer is allocated from PSRAM when it is available, while the two partial
rendering buffers are allocated from internal RAM.

## Hardware topology

```text
Core2 v1.3
  └─ M-Bus ─ Audio Module M144 (Configuration A)
               └─ M-Bus passthrough ─ Faces Bottom3 ─ Faces Gamepad3
```

The audio module's ES8388 codec is controlled at I²C address `0x10`. Faces
Gamepad3 uses `0x08`; the audio module controller remains at `0x33`. Core2's
PMIC and touch controller use their normal internal-bus addresses, so these
devices do not overlap.

Core2's SD card is initialized through the M5Unified pin map:

| Signal | GPIO |
| --- | ---: |
| SCK | 18 |
| MISO | 38 |
| MOSI | 23 |
| CS | 4 |

The Audio Module I²S output pins resolved through the M5Unified M-Bus map are:

| Signal | GPIO |
| --- | ---: |
| BCLK | 19 |
| LRCK | 27 |
| DOUT | 2 |
| MCLK | 0 |

`M5.config()` disables Core2's built-in speaker and microphone. LoFiBox's
existing ESP32-audioI2S decoder is the sole I²S-driver owner; the backend uses
only the Audio Module library's `ES8388` codec controller over M5Unified I²C.
Do not initialize `M5ModuleAudio` in this application, because its `begin()`
also initializes an I²S driver.

The Core2 environment configures ESP32-audioI2S with eight 256-sample DMA
buffers instead of the library default of sixteen 512-sample buffers. I²S DMA
must reside in internal RAM, not PSRAM; this reduces the allocation from about
32 KiB to about 8 KiB while retaining roughly 43–46 ms of 16-bit stereo output
buffering at 48/44.1 kHz. The full input/decode buffer remains in PSRAM.

## Controls

| Gamepad3 control | LoFiBox action |
| --- | --- |
| D-pad | LVGL navigation |
| A | Confirm / Enter |
| B | Back / Escape |
| Start | Play / Pause |
| Select | Cycle playback mode |

D-pad navigation produces one event on the press edge. Holding a direction
begins repeat after 350 ms and repeats every 80 ms, so a single press does not
skip through a list.

## Audio sample-rate synchronization

The decoder can change I²S rate with the current file. Each non-zero rate
change observed by the player is forwarded through `BoardBase` to the Core2
backend, which configures ES8388 for 8, 11.025, 16, 24, 32, 44.1, or 48 kHz.
Unsupported rates leave the codec's previous configuration intact.

## Power and known limitations

Shutdown delegates to `M5.Power.powerOff()` and charging state and battery
percentage are reported by the Core2 AXP192. Because Faces Bottom3's battery
and TP4057 charger are outside the original Core2 battery base arrangement,
the displayed percentage is a Core2 PMIC estimate rather than a calibrated
Bottom3 state-of-charge measurement.

Mechanical fit is a hardware prerequisite, not something firmware can prove.
Before regular use, verify that the Audio Module and Faces Bottom3 stack
correctly, all M-Bus contacts are fully seated, and the combined height does
not load the Core2 connector.

## On-device acceptance checklist

1. Boot to the loading screen and confirm all main UI pages render at 320×240.
2. Scan `/music`, browse the library, and continuously read files for at least
   10 minutes without SD errors or display corruption.
3. Verify all eight Gamepad3 buttons, including one-shot press and directional
   repeat behavior.
4. Play MP3 and WAV material at 44.1 and 48 kHz; confirm that rate transitions
   do not cause a codec lockup or persistent noise.
5. Run the full combination test—SD decoding, EQ, cover loading, display
   refresh, and Gamepad polling—for at least one hour. Monitor serial output
   for I²S underruns, SD timeouts, heap/PSRAM growth, missed inputs, and Guru
   Meditation errors.
