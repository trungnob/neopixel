// pattern_103_rain_drops.cpp
#include "../patterns.h"

// Rain Drops - Droplets falling down
void pattern_rain_drops(CRGB* leds, int activeLeds, uint8_t& hue) {
// Shift everything down
          for(int y=0; y<GRID_HEIGHT-1; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              int ledAbove = XY(x, y+1);
              if (led >= 0 && ledAbove >= 0) {
                leds[led] = leds[ledAbove];
                leds[led].fadeToBlackBy(10);
              }
            }
          }
          // Add new drops at top
          for(int x=0; x<GRID_WIDTH; x++) {
            if (random8() < 30) {
              int led = XY(x, GRID_HEIGHT-1);
              if (led >= 0) leds[led] = CHSV(160, 255, 255);
            }
          }
}
