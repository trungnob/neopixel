// pattern_109_plasma_2d.cpp
#include "../patterns.h"

// Plasma 2D - Full 2D plasma effect
void pattern_plasma_2d(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              uint8_t wave1 = sin8((x * 8) + (hue));
              uint8_t wave2 = sin8((y * 16) + (hue * 2));
              uint8_t wave3 = sin8(((x + y) * 6) + (hue * 3));
              uint8_t combined = (wave1 + wave2 + wave3) / 3;
              leds[led] = CHSV(combined, 255, 255);
            }
          }
        }
        hue++;
}
