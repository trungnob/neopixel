// pattern_112_wave_pool.cpp
#include "../patterns.h"

// Wave Pool - Horizontal waves perfect for 1m strips
void pattern_wave_pool(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              uint8_t wave1 = sin8((x * 3) + (hue * 2));
              uint8_t wave2 = sin8((x * 2) - (hue * 3) + (y * 20));
              uint8_t brightness = (wave1 + wave2) / 2;
              leds[led] = CHSV(160, 255, brightness);
            }
          }
        }
        hue++;
}
