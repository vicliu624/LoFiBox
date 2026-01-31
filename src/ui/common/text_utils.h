#pragma once

#include <Arduino.h>

namespace lofi::ui::text
{
String single_line(const String& input);
String single_line(const char* input);
String truncate_utf8(const String& input, size_t max_chars);
} // namespace lofi::ui::text
