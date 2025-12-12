// pattern_114_lava_lamp.cpp
#include "../patterns.h"

// Lava Lamp 2D - Aspect-ratio corrected blobs
void pattern_lava_lamp(CRGB* leds, int activeLeds, uint8_t& hue) {
for(int y=0; y<GRID_HEIGHT; y++) {
          for(int x=0; x<GRID_WIDTH; x++) {
            int led = XY(x, y);
            if (led >= 0) {
              // Correct for aspect ratio in noise calculation
              int scaleX = 10;
              int scaleY = 10;
              if (ASPECT_RATIO > 2.0) {
                 scaleY = 70; // Stretch Y for 9x144
              }
              
              uint8_t blob1 = inoise8(x * scaleX, y * scaleY, hue * 2);
              uint8_t blob2 = inoise8(x * (scaleX + 5), y * (scaleY + 30), hue * 3 + 10000);
              uint8_t combined = (blob1 + blob2) / 2;
              leds[led] = ColorFromPalette(LavaColors_p, combined);
            }
          }
        }
        hue++;
}
