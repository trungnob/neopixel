// pattern_111_game_of_life.cpp
#include "../patterns.h"

// Game of Life - Conway's cellular automaton
void pattern_game_of_life(CRGB* leds, int activeLeds, uint8_t& hue) {
static uint8_t grid[GRID_HEIGHT][GRID_WIDTH];
          static uint8_t nextGrid[GRID_HEIGHT][GRID_WIDTH];
          static unsigned long lastUpdate = 0;
          static bool initialized111 = false;

          if (!initialized111) {
            // Random initial state
            for(int y=0; y<GRID_HEIGHT; y++) {
              for(int x=0; x<GRID_WIDTH; x++) {
                grid[y][x] = random8(100) < 30 ? 1 : 0;
              }
            }
            initialized111 = true;
          }

          if (millis() - lastUpdate > 200) {
            // Apply Game of Life rules
            for(int y=0; y<GRID_HEIGHT; y++) {
              for(int x=0; x<GRID_WIDTH; x++) {
                // Count neighbors
                int neighbors = 0;
                for(int dy=-1; dy<=1; dy++) {
                  for(int dx=-1; dx<=1; dx++) {
                    if (dx==0 && dy==0) continue;
                    int ny = (y + dy + GRID_HEIGHT) % GRID_HEIGHT;
                    int nx = (x + dx + GRID_WIDTH) % GRID_WIDTH;
                    neighbors += grid[ny][nx];
                  }
                }
                // Conway's rules
                if (grid[y][x] == 1) {
                  nextGrid[y][x] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
                } else {
                  nextGrid[y][x] = (neighbors == 3) ? 1 : 0;
                }
              }
            }
            // Copy next to current
            memcpy(grid, nextGrid, sizeof(grid));
            lastUpdate = millis();
          }

          // Draw to LEDs
          for(int y=0; y<GRID_HEIGHT; y++) {
            for(int x=0; x<GRID_WIDTH; x++) {
              int led = XY(x, y);
              if (led >= 0) {
                if (grid[y][x]) {
                  leds[led] = CHSV(hue, 255, 255);
                } else {
                  leds[led] = CRGB::Black;
                }
              }
            }
          }
          hue++;
}
