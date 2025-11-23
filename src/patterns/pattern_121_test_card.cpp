// pattern_121_test_card.cpp
#include "../patterns.h"

// Test Card - simple moving hue grid to validate orientation
void pattern_test_card(CRGB* leds, int activeLeds, uint8_t& hue) {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      int idx = XY(x, y);
      if (idx < 0 || idx >= activeLeds) continue;
      uint8_t sat = 200;
      uint8_t val = 180 + ((x + y) % 70); // subtle brightness variation
      leds[idx] = CHSV(hue + x * 2 + y * 8, sat, val);
    }
  }
  hue++;
}
