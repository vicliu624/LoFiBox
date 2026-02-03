#pragma once

#include <cstdint>

namespace display {

struct ScreenSize {
  int width;
  int height;
};

inline ScreenSize screen_size() {
#if defined(SCREEN_WIDTH) && defined(SCREEN_HEIGHT)
  return {SCREEN_WIDTH, SCREEN_HEIGHT};
#else
  return {0, 0};
#endif
}

} // namespace display