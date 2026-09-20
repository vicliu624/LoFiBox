# Real-time lyrics

LoFiBox supports the same lyrics workflow on T-LoRa Pager and M5Stack Core2.

## Connect Wi-Fi

1. Open **Settings** and select **Wi-Fi**.
2. Select **Wi-Fi** to turn the radio on or off.
3. Select **Scan networks**, then choose an SSID from the results, or choose
   **Other network** to enter an SSID manually.
4. Enter the password and select **SAVE** to connect.

The player stores the selected SSID and password in its own NVS storage and
reconnects after later boots.  The credential editor works entirely on the
device: Pager accepts physical keyboard text, while Core2 uses the on-screen
character grid with its Gamepad.

## Download and use lyrics

With a song selected in **Now Playing**:

- Press **Up** to request synchronized lyrics for the current title and artist.
- Press **Down** to toggle the Now Playing lyrics mode.  Press **Down** again
  (or **Back**) to return to the player view.
- In lyrics mode, **Left/Right** select the previous/next track and
  **Enter** toggles playback.

Lyrics are retrieved only when the user presses Download.  A successful result
is stored on the SD card in `/lyrics/`, keyed by the music-file path, so it is
available offline afterwards.  A sidecar `.lrc` file alongside an audio file
takes precedence over the cached download.

Downloading is deliberately an explicit foreground operation.  If audio is
playing, it is paused for the short network request and resumed afterwards;
this prevents a slow Wi-Fi response from starving the audio DMA buffer and
causing an audible glitch.

The lyric view supports standard timestamped LRC lines, including multiple
timestamps on a single line.  It updates from the player time at 100 ms cadence
and keeps the active line centred while neighbouring lines scroll around it.
