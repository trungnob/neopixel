// pattern_107_diagonal_sweep.cpp
#include "../patterns.h"

// Diagonal Sweep - Diagonal lines moving
void pattern_diagonal_sweep(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              uint8_t dist = (x + y*10 + hue*2) % 256;
              leds[led] = CHSV(dist, 255, sin8(dist));
            }
          }
        }
        hue++;
}
