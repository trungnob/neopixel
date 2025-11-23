// pattern_115_ripple_2d.cpp
#include "../patterns.h"

// 2D Ripple - Aspect-ratio corrected circles
void pattern_ripple_2d(CRGB* leds, int activeLeds, uint8_t& hue) {
static int centerX = GRID_WIDTH / 2;
          static int centerY = GRID_HEIGHT / 2;

          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              if (led >= 0) {
                // Calculate distance with aspect ratio correction
                float dx = (x - centerX);
                float dy = (y - centerY) * ASPECT_RATIO;
                float dist = sqrt(dx*dx + dy*dy);
                uint8_t brightness = sin8((dist * 10) - (hue * 3));
                leds[led] = CHSV(hue + (dist * 2), 255, brightness);
              }
            }
          }
          hue += 2;

          EVERY_N_SECONDS(5) {
            centerX = random16(GRID_WIDTH);
            centerY = random16(GRID_HEIGHT);
          }
}
