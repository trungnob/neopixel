// pattern_108_vertical_wave.cpp
#include "../patterns.h"

// Vertical Wave - Sine wave across strips
void pattern_vertical_wave(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          uint8_t yPos = beatsin8(15, 0, GRID_WIDTH-1, 0, y*20);
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              int dist = abs(x - yPos);
              uint8_t brightness = dist < 5 ? 255 - (dist*50) : 0;
              leds[led] = CHSV(hue + y*28, 255, brightness);
            }
          }
        }
        hue++;
}
