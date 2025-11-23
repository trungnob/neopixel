// pattern_105_scanning_lines.cpp
#include "../patterns.h"

// Scanning Lines - Horizontal lines moving up/down
void pattern_scanning_lines(CRGB* leds, int activeLeds, uint8_t& hue) {
static int scanLine = 0;
          fill_solid(leds, activeLeds, CRGB::Black);
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, scanLine);
            if (led >= 0) leds[led] = CHSV(hue, 255, 255);
            // Add trail
            int led2 = XY(x, (scanLine + 1) % GRID_HEIGHT);
            if (led2 >= 0) leds[led2] = CHSV(hue, 255, 128);
          }
          EVERY_N_MILLISECONDS(100) {
            scanLine = (scanLine + 1) % GRID_HEIGHT;
            hue += 5;
          }
}
