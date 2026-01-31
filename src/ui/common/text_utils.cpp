#include "ui/common/text_utils.h"

#include <lvgl.h>

namespace lofi::ui::text
{
String single_line(const String& input)
{
    String out = input;
    for (size_t i = 0; i < out.length(); ++i) {
        char c = out[i];
        if (c == '\n' || c == '\r' || c == '\t') {
            out.setCharAt(i, ' ');
        }
    }
    return out;
}

String single_line(const char* input)
{
    if (!input) {
        return String();
    }
    return single_line(String(input));
}

String truncate_utf8(const String& input, size_t max_chars)
{
    if (max_chars == 0) {
        return "";
    }
    const char* text = input.c_str();
    size_t total_chars = lv_text_get_encoded_length(text);
    if (total_chars <= max_chars) {
        return input;
    }
    size_t keep = max_chars;
    if (max_chars > 3) {
        keep = max_chars - 3;
    }
    uint32_t i = 0;
    size_t count = 0;
    while (text[i] && count < keep) {
        lv_text_encoded_next(text, &i);
        ++count;
    }
    size_t bytes = static_cast<size_t>(i);
    String out = input.substring(0, bytes);
    if (max_chars > 3) {
        out += "...";
    }
    return out;
}
} // namespace lofi::ui::text
