// pattern_117_side_fire.cpp
#include "../patterns.h"

// Side Fire - Fire from left and right edges
void pattern_side_fire(CRGB* leds, int activeLeds, uint8_t& hue) {
static byte heatLeft[GRID_HEIGHT][GRID_WIDTH/2];
          static byte heatRight[GRID_HEIGHT][GRID_WIDTH/2];

          // Cool down
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH/2; x++) {
              heatLeft[y][x] = qsub8(heatLeft[y][x], random8(0, 15));
              heatRight[y][x] = qsub8(heatRight[y][x], random8(0, 15));
            }
          }

          // Heat spreads inward
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=GRID_WIDTH/2-1; x>0; x--) {
              heatLeft[y][x] = (heatLeft[y][x-1] + heatLeft[y][x]) / 2;
              heatRight[y][x] = (heatRight[y][x-1] + heatRight[y][x]) / 2;
            }
          }

          // Add new fire at edges
          for(int y=0; y<GRID_HEIGHT; y++) {
            if(random8() < 120) {
              heatLeft[y][0] = qadd8(heatLeft[y][0], random8(160, 255));
              heatRight[y][0] = qadd8(heatRight[y][0], random8(160, 255));
            }
          }

          // Draw to LEDs
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH/2; x++) {
              int ledLeft = XY(x, y);
              int ledRight = XY(GRID_WIDTH - 1 - x, y);
              if (ledLeft >= 0) leds[ledLeft] = HeatColor(heatLeft[y][x]);
              if (ledRight >= 0) leds[ledRight] = HeatColor(heatRight[y][x]);
            }
          }
}
