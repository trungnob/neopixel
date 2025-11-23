// pattern_106_checkerboard.cpp
#include "../patterns.h"

// Checkerboard - Classic 2D pattern
void pattern_checkerboard(CRGB* leds, int activeLeds, uint8_t& hue) {
int cellSize = 8;
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              if (led >= 0) {
                bool isWhite = ((x/cellSize) + (y)) % 2 == (hue/50) % 2;
                if (isWhite) {
                  leds[led] = CHSV(hue, 255, 255);
                } else {
                  leds[led] = CRGB::Black;
                }
              }
            }
          }
          hue++;
}
