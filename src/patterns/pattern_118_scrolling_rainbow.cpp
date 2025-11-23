// pattern_118_scrolling_rainbow.cpp
#include "../patterns.h"

// Scrolling Rainbow - Smooth horizontal scroll
void pattern_scrolling_rainbow(CRGB* leds, int activeLeds, uint8_t& hue) {
static int scrollPos = 0;
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              if (led >= 0) {
                uint8_t colorIndex = ((x + scrollPos) * 256 / GRID_WIDTH) + (y * 20);
                leds[led] = CHSV(colorIndex, 255, 255);
              }
            }
          }
          EVERY_N_MILLISECONDS(50) {
            scrollPos = (scrollPos + 1) % GRID_WIDTH;
          }
}
