#include "app/network.h"

#include <Preferences.h>
#include <WiFi.h>

namespace app::network {
namespace {
constexpr char kPrefsNamespace[] = "wifi";
constexpr char kEnabledKey[] = "enabled";
constexpr char kSsidKey[] = "ssid";
constexpr char kPasswordKey[] = "password";
constexpr uint32_t kReconnectIntervalMs = 15000;

ScanResult s_results[kMaxScanResults];
uint8_t s_result_count = 0;
String s_ssid;
bool s_enabled = false;
bool s_scanning = false;
bool s_scan_changed = false;
uint32_t s_last_connect_attempt_ms = 0;

void load_saved_network() {
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, true))
    return;
  s_enabled = prefs.getBool(kEnabledKey, false);
  s_ssid = prefs.getString(kSsidKey, "");
  prefs.end();
}

void connect_saved_network() {
  if (!s_enabled || s_ssid.isEmpty())
    return;
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, true))
    return;
  const String password = prefs.getString(kPasswordKey, "");
  prefs.end();
  WiFi.mode(WIFI_STA);
  WiFi.begin(s_ssid.c_str(), password.c_str());
  s_last_connect_attempt_ms = millis();
}

void finish_scan(int found) {
  s_result_count = 0;
  if (found > 0) {
    for (int i = 0; i < found && s_result_count < kMaxScanResults; ++i) {
      const String ssid = WiFi.SSID(i);
      if (ssid.isEmpty())
        continue;
      bool duplicate = false;
      for (uint8_t j = 0; j < s_result_count; ++j) {
        if (s_results[j].ssid == ssid) {
          duplicate = true;
          break;
        }
      }
      if (duplicate)
        continue;
      ScanResult &result = s_results[s_result_count++];
      result.ssid = ssid;
      result.rssi = WiFi.RSSI(i);
      result.secured = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
    }
  }
  WiFi.scanDelete();
  s_scanning = false;
  s_scan_changed = true;
  Serial.printf("[WIFI] scan complete networks=%u\n",
                static_cast<unsigned>(s_result_count));
}
} // namespace

void init() {
  load_saved_network();
  // Do not leave the radio powered in STA mode merely because Wi-Fi is
  // enabled in Settings but no network has been selected yet. Scanning and
  // connecting explicitly re-enable it when the user asks for networking.
  if (!s_enabled || s_ssid.isEmpty()) {
    WiFi.mode(WIFI_OFF);
    return;
  }
  WiFi.mode(WIFI_STA);
  connect_saved_network();
}

void tick() {
  if (!s_enabled)
    return;
  if (s_scanning) {
    const int state = WiFi.scanComplete();
    if (state >= 0)
      finish_scan(state);
  }
  if (!s_ssid.isEmpty() && !s_scanning && WiFi.status() != WL_CONNECTED &&
      millis() - s_last_connect_attempt_ms >= kReconnectIntervalMs) {
    connect_saved_network();
  }
}

bool enabled() { return s_enabled; }

void set_enabled(bool value) {
  if (s_enabled == value)
    return;
  s_enabled = value;
  Preferences prefs;
  if (prefs.begin(kPrefsNamespace, false)) {
    prefs.putBool(kEnabledKey, s_enabled);
    prefs.end();
  }
  s_scanning = false;
  s_scan_changed = true;
  if (!s_enabled) {
    WiFi.scanDelete();
    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);
    return;
  }
  WiFi.mode(WIFI_STA);
  connect_saved_network();
}

bool connected() { return s_enabled && WiFi.status() == WL_CONNECTED; }

String status_label() {
  if (!s_enabled)
    return "Off";
  if (connected())
    return "Connected";
  if (s_scanning)
    return "Scanning...";
  return s_ssid.isEmpty() ? "On" : "Connecting...";
}

String saved_ssid() { return s_ssid; }

bool start_scan() {
  if (!s_enabled || s_scanning)
    return false;
  WiFi.mode(WIFI_STA);
  WiFi.scanDelete();
  const int started = WiFi.scanNetworks(true, true);
  if (started == WIFI_SCAN_FAILED) {
    s_scan_changed = true;
    return false;
  }
  s_scanning = true;
  s_scan_changed = true;
  return true;
}

bool scan_in_progress() { return s_scanning; }
uint8_t scan_count() { return s_result_count; }
const ScanResult *scan_result(uint8_t index) {
  return index < s_result_count ? &s_results[index] : nullptr;
}
bool take_scan_changed() {
  const bool changed = s_scan_changed;
  s_scan_changed = false;
  return changed;
}

bool connect(const String &ssid, const String &password) {
  if (!s_enabled || ssid.isEmpty())
    return false;
  Preferences prefs;
  if (!prefs.begin(kPrefsNamespace, false))
    return false;
  prefs.putString(kSsidKey, ssid);
  prefs.putString(kPasswordKey, password);
  prefs.end();
  s_ssid = ssid;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  s_last_connect_attempt_ms = millis();
  Serial.printf("[WIFI] connecting ssid=%s\n", s_ssid.c_str());
  return true;
}

} // namespace app::network
