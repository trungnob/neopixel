// pattern_111_game_of_life.cpp
#include "../patterns.h"

// Game of Life - Conway's cellular automaton
void pattern_game_of_life(CRGB* leds, int activeLeds, uint8_t& hue) {
// Optimize memory: 32x32 (1024) or 9x144 (1296). 
// We can't afford 32x144 (4608 bytes) x 2 grids = 9KB RAM!
// Let's use a single buffer size that fits the largest pixel count (1296 for 9x144)
// and map it dynamically.
static uint8_t grid[1296];      // Max LEDs (9x144 = 1296)
static uint8_t nextGrid[1296];
          static unsigned long lastUpdate = 0;
          static bool initialized111 = false;

          if (!initialized111) {
            // Random initial state
            for(int i=0; i<activeLeds; i++) {
              grid[i] = random8(100) < 30 ? 1 : 0;
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
                    
                    // Map 2D neighbor coord to 1D index
                    // Note: This assumes row-major logical layout for the grid buffer
                    int idx = ny * GRID_WIDTH + nx;
                    if (idx < 1296) neighbors += grid[idx];
                  }
                }
                // Conway's rules
                int currentIdx = y * GRID_WIDTH + x;
                if (currentIdx < 1296) {
                  if (grid[currentIdx] == 1) {
                    nextGrid[currentIdx] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
                  } else {
                    nextGrid[currentIdx] = (neighbors == 3) ? 1 : 0;
                  }
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
                int gridIdx = y * GRID_WIDTH + x;
                if (gridIdx < 1296) {
                  if (grid[gridIdx]) {
                    leds[led] = CHSV(hue, 255, 255);
                  } else {
                    leds[led] = CRGB::Black;
                  }
                }
              }
            }
          }
          hue++;
}
