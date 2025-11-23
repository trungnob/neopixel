// pattern_101_vertical_ripple.cpp
#include "../patterns.h"

// Vertical Ripple - Waves moving vertically
void pattern_vertical_ripple(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          uint8_t brightness = beatsin8(20, 0, 255, 0, y*32);
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) leds[led] = CHSV(hue, 255, brightness);
          }
        }
        hue++;
}
