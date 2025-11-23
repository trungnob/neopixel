// pattern_116_starfield.cpp
#include "../patterns.h"

// Starfield Parallax - Stars moving at different speeds
void pattern_starfield(CRGB* leds, int activeLeds, uint8_t& hue) {
static float stars[20][3]; // x, y, speed
          static bool initialized116 = false;

          if (!initialized116) {
            for(int i=0; i<20; i++) {
              stars[i][0] = random16(GRID_WIDTH);
              stars[i][1] = random16(GRID_HEIGHT);
              stars[i][2] = random8(1, 5) / 10.0;
            }
            initialized116 = true;
          }

          fadeToBlackBy(leds, activeLeds, 30);

          for(int i=0; i<20; i++) {
            int led = XY((int)stars[i][0], (int)stars[i][1]);
            if (led >= 0) {
              uint8_t brightness = 100 + (stars[i][2] * 300);
              leds[led] = CRGB(brightness, brightness, brightness);
            }

            // Move star
            stars[i][0] -= stars[i][2];
            if (stars[i][0] < 0) {
              stars[i][0] = GRID_WIDTH - 1;
              stars[i][1] = random16(GRID_HEIGHT);
            }
          }
}
