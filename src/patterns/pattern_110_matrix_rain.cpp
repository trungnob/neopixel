// pattern_110_matrix_rain.cpp
#include "../patterns.h"

// Matrix Rain 2D - Proper Matrix effect with columns
void pattern_matrix_rain(CRGB* leds, int activeLeds, uint8_t& hue) {
static uint8_t drops[GRID_WIDTH];
          static bool initialized110 = false;

          if (!initialized110) {
            for(int x=0; x<GRID_WIDTH; x++) {
              drops[x] = random8(GRID_HEIGHT);
            }
            initialized110 = true;
          }

          fadeToBlackBy(leds, activeLeds, 40);

          for(int x=0; x<GRID_WIDTH; x++) {
            // Draw the head (bright green)
            int led = XY(x, drops[x]);
            if (led >= 0) leds[led] = CRGB::Green;

            // Move drop down
            if (random8() < 100) {
              drops[x] = (drops[x] - 1 + GRID_HEIGHT) % GRID_HEIGHT;
            }
          }
}
