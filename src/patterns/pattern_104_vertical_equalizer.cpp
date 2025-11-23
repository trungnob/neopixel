// pattern_104_vertical_equalizer.cpp
#include "../patterns.h"

// Vertical Equalizer - Each strip is a bar
void pattern_vertical_equalizer(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          int barHeight = beatsin8(40 + y*5, 0, GRID_WIDTH);
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              if (x < barHeight) {
                uint8_t barHue = (y * 255) / GRID_HEIGHT;
                leds[led] = CHSV(barHue, 255, 255);
              } else {
                leds[led] = CRGB::Black;
              }
            }
          }
        }
}
