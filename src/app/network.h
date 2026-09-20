#pragma once

#include <Arduino.h>

namespace app::network {

constexpr uint8_t kMaxScanResults = 16;

struct ScanResult {
  String ssid;
  int32_t rssi = 0;
  bool secured = true;
};

void init();
void tick();

bool enabled();
void set_enabled(bool value);
bool connected();
String status_label();
String saved_ssid();

bool start_scan();
bool scan_in_progress();
uint8_t scan_count();
const ScanResult *scan_result(uint8_t index);
// Returns true once after an asynchronous scan has completed.
bool take_scan_changed();

bool connect(const String &ssid, const String &password);

} // namespace app::network
