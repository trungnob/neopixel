// pattern_102_fire_rising.cpp
#include "../patterns.h"

// 2D Fire Rising - Fire effect rising from bottom
void pattern_fire_rising(CRGB* leds, int activeLeds, uint8_t& hue) {
static byte heat2d[GRID_HEIGHT][GRID_WIDTH];
          // Cool down every cell
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              heat2d[y][x] = qsub8(heat2d[y][x], random8(0, 20));
            }
          }
          // Heat rises
          for(int y=GRID_HEIGHT-1; y>0; y--) {
            for(int x=0; x<GRID_WIDTH; x++) {
              heat2d[y][x] = (heat2d[y-1][x] + heat2d[y][x]) / 2;
            }
          }
          // Add new fire at bottom
          for(int x=0; x<GRID_WIDTH; x++) {
            if(random8() < 120) {
              heat2d[0][x] = qadd8(heat2d[0][x], random8(160, 255));
            }
          }
          // Convert to LED colors
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              if (led >= 0) leds[led] = HeatColor(heat2d[y][x]);
            }
          }
}
