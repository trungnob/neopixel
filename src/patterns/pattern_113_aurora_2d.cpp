// pattern_113_aurora_2d.cpp
#include "../patterns.h"

// Aurora 2D - Optimized for horizontal strips
void pattern_aurora_2d(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              // Horizontal waves with vertical variation
              uint8_t wave1 = sin8((x * 2) + (hue * 3));
              uint8_t wave2 = sin8((x * 3) - (hue * 2) + (y * 30));
              uint8_t colorVal = 80 + ((wave1 + wave2) / 8);
              uint8_t brightness = (wave1 + wave2) / 2;
              leds[led] = CHSV(colorVal, 200, brightness);
            }
          }
        }
        hue++;
}
