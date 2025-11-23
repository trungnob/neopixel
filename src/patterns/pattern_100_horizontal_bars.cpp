// pattern_100_horizontal_bars.cpp
#include "../patterns.h"

// Horizontal Bars - Each strip a different cycling color
void pattern_horizontal_bars(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          uint8_t stripHue = (hue + y * 28) % 256;
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) leds[led] = CHSV(stripHue, 255, 255);
          }
        }
        hue++;
}
