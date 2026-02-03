#pragma once

#include <lvgl.h>

void beginLvglHelper();
void lvHelperTick();
const uint16_t *lvHelperGetFrameBuffer(uint16_t *width, uint16_t *height);
uint32_t lvHelperGetBacklightTimeoutMs();
uint32_t lvHelperGetSleepTimeoutMs();
void lvHelperSetBacklightTimeoutMs(uint32_t timeout_ms);
void lvHelperSetSleepTimeoutMs(uint32_t timeout_ms);
void lvHelperFormatTimeout(char *out, size_t len, uint32_t timeout_ms);
